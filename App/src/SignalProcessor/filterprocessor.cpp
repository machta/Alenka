#include "filterprocessor.h"

#include "../options.h"
#include "../error.h"

#include <cassert>

using namespace std;

FilterProcessor::FilterProcessor(unsigned int M, unsigned int blockWidth, unsigned int blockHeight, OpenCLContext* context) : M(M), width(blockWidth), height(blockHeight)
{
	assert(blockWidth%4 == 0);

	cl_int err;
	clfftStatus errFFT;

	FILE* file = fopen("kernels.cl", "rb");
	checkNotErrorCode(file, nullptr, "File 'kernels.cl' could not be opened.");

	OpenCLProgram program(file, context);

	int ierr = fclose(file);
	checkErrorCode(ierr, 0, "fclose()");

	filterKernel = program.createKernel("filter");
	zeroKernel = program.createKernel("zero");

	cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_VERSION_1_2
	flags |= CL_MEM_HOST_WRITE_ONLY;
#endif
#endif

	filterBuffer = clCreateBuffer(context->getCLContext(), flags, (width + 4)*sizeof(float), nullptr, &err);
	checkClErrorCode(err, "clCreateBuffer");

	// Construct the fft plans.
	size_t size = width;
	size_t bufferDistance = size + 4;

	errFFT = clfftCreateDefaultPlan(&fftPlan, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	clfftSetLayout(fftPlan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	clfftSetResultLocation(fftPlan, CLFFT_INPLACE);
	clfftSetPlanBatchSize(fftPlan, 1);
	//clfftSetPlanDistance(fftPlan, bufferDistance, bufferDistance/2);

	errFFT = clfftCreateDefaultPlan(&fftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	clfftSetLayout(fftPlanBatch, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	clfftSetResultLocation(fftPlanBatch, CLFFT_INPLACE);
	clfftSetPlanBatchSize(fftPlanBatch, height);
	clfftSetPlanDistance(fftPlanBatch, bufferDistance, bufferDistance/2);

	errFFT = clfftCreateDefaultPlan(&ifftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	clfftSetLayout(ifftPlanBatch, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	clfftSetResultLocation(ifftPlanBatch, CLFFT_INPLACE);
	clfftSetPlanBatchSize(ifftPlanBatch, height);
	clfftSetPlanDistance(ifftPlanBatch, bufferDistance/2, bufferDistance);
}

FilterProcessor::~FilterProcessor()
{
	cl_int err;

	err = clReleaseKernel(filterKernel);
	checkClErrorCode(err, "clReleaseKernel()");

	err = clReleaseKernel(zeroKernel);
	checkClErrorCode(err, "clReleaseKernel()");

	err = clReleaseMemObject(filterBuffer);
	checkClErrorCode(err, "clReleaseMemObject()");

	clfftStatus errFFT;
	errFFT = clfftDestroyPlan(&fftPlan);
	checkClfftErrorCode(errFFT, "clfftDestroyPlan()");
	errFFT = clfftDestroyPlan(&fftPlanBatch);
	checkClfftErrorCode(errFFT, "clfftDestroyPlan()");
	errFFT = clfftDestroyPlan(&ifftPlanBatch);
	checkClfftErrorCode(errFFT, "clfftDestroyPlan()");
}

void FilterProcessor::change(Filter* filter)
{
	coefficients.clear();

	auto tmp = filter->computeCoefficients();

	for (unsigned int i = 0; i < M; ++i)
	{
		coefficients.push_back(static_cast<float>(tmp[i]));
	}

	coefficientsChanged = true;
}

void FilterProcessor::process(cl_mem buffer, cl_command_queue queue)
{
	cl_int err;
	clfftStatus errFFT;

	if (coefficientsChanged)
	{
		// Update values in the filterBuffer.
		logToFile("Updating filterBuffer.");

		coefficientsChanged = false;

		// This section is disabled because a bug in the implementation of clEnqueueFillBuffer().
//#if CL_VERSION_1_2
//		float zero = 0;
//		err = clEnqueueFillBuffer(queue, filterBuffer, &zero, sizeof(zero), 0, width + 4, 0, nullptr, nullptr);
//		checkClErrorCode(err, "clEnqueueFillBuffer()");
//#else
		err = clSetKernelArg(zeroKernel, 0, sizeof(cl_mem), &filterBuffer);
		checkClErrorCode(err, "clSetKernelArg()");

		size_t globalWorkSize = (width + 4)/4;
		size_t globalWorkOffset = M/4;
		err = clEnqueueNDRangeKernel(queue, zeroKernel, 1, &globalWorkOffset, &globalWorkSize, nullptr, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueNDRangeKernel()");
//#endif

		err = clEnqueueWriteBuffer(queue, filterBuffer, CL_FALSE, 0, M*sizeof(float), coefficients.data(), 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueWriteBuffer()");

		errFFT = clfftEnqueueTransform(fftPlan, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &filterBuffer, nullptr, nullptr);
		checkClfftErrorCode(errFFT, "clfftEnqueueTransform");

		printBuffer("filterBuffer.txt", filterBuffer, queue);
	}

	// FFT.
	errFFT = clfftEnqueueTransform(fftPlanBatch, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &buffer, nullptr, nullptr);
	checkClfftErrorCode(errFFT, "clfftEnqueueTransform");

	printBuffer("after_fft.txt", buffer, queue);

	// Multiply.
	err = clSetKernelArg(filterKernel, 0, sizeof(cl_mem), &buffer);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(filterKernel, 1, sizeof(cl_mem), &filterBuffer);
	checkClErrorCode(err, "clSetKernelArg()");

	size_t globalWorkSize[2] = {height, (width + 4)/4};

	err = clEnqueueNDRangeKernel(queue, filterKernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueNDRangeKernel()");

	printBuffer("after_multiply.txt", buffer, queue);

	// IFFT.
	errFFT = clfftEnqueueTransform(ifftPlanBatch, CLFFT_BACKWARD, 1, &queue, 0, nullptr, nullptr, &buffer, nullptr, nullptr);
	checkClfftErrorCode(errFFT, "clfftEnqueueTransform");
}
