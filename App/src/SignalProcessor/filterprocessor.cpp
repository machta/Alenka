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

    FILE* file = fopen(PROGRAM_OPTIONS["kernels"].as<string>().c_str(), "rb");
    checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["kernels"].as<string>() << "' could not be opened.");

	program = new OpenCLProgram(file, context);

	fclose(file);

	filterKernel = program->createKernel("filter");

	filterBuffer = clCreateBuffer(context->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, (width + 2)*height*sizeof(float), nullptr, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateBuffer");

	coefficients = new float[M];

	// Construct the fft plans.
	size_t size = width;
	size_t outBufferDistance = size + 4;

	errFFT = clfftCreateDefaultPlan(&fftPlan, context->getCLContext(), CLFFT_1D, &size);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftCreateDefaultPlan()");
	clfftSetLayout(fftPlan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	clfftSetResultLocation(fftPlan, CLFFT_INPLACE);

	errFFT = clfftCreateDefaultPlan(&fftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftCreateDefaultPlan()");
	clfftSetLayout(fftPlanBatch, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	clfftSetResultLocation(fftPlanBatch, CLFFT_OUTOFPLACE);
	clfftSetPlanBatchSize(fftPlanBatch, height);
	clfftSetPlanDistance(fftPlanBatch, size, outBufferDistance/2);

	errFFT = clfftCreateDefaultPlan(&ifftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftCreateDefaultPlan()");
	clfftSetLayout(ifftPlanBatch, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	clfftSetResultLocation(fftPlan, CLFFT_INPLACE);
	clfftSetPlanBatchSize(ifftPlanBatch, height);
	clfftSetPlanDistance(ifftPlanBatch, outBufferDistance/2, outBufferDistance);
}

FilterProcessor::~FilterProcessor()
{
	delete program;
	delete[] coefficients;

	clfftStatus errFFT;

	errFFT = clfftDestroyPlan(&fftPlan);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftDestroyPlan()");
	errFFT = clfftDestroyPlan(&fftPlanBatch);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftDestroyPlan()");
	errFFT = clfftDestroyPlan(&ifftPlanBatch);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftDestroyPlan()");
}

void FilterProcessor::change(Filter* filter)
{
    double* tmp = filter->computeCoefficients();

	for (unsigned int i = 0; i < M; ++i)
	{
        coefficients[i] = static_cast<float>(tmp[i]);
	}

	coefficientsChanged = true;
}

void FilterProcessor::process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue)
{
	cl_int err;
	clfftStatus errFFT;

	if (coefficientsChanged)
	{
		coefficientsChanged = false;

		// Update values in the filterBuffer.
		float zero = 0;
		err = clEnqueueFillBuffer(queue, filterBuffer, &zero, sizeof(zero), 0, width, 0, nullptr, nullptr);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueFillBuffer()");

		err = clEnqueueWriteBuffer(queue, filterBuffer, CL_FALSE, 0, M, coefficients, 0, nullptr, nullptr);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");

		errFFT = clfftEnqueueTransform(fftPlan, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &filterBuffer, nullptr, nullptr);
		checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftEnqueueTransform");
	}

	// FFT.
	errFFT = clfftEnqueueTransform(fftPlanBatch, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &inBuffer, &outBuffer, nullptr);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftEnqueueTransform");

	// Multiply.
	err = clSetKernelArg(filterKernel, 0, sizeof(cl_mem), &outBuffer);
	checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

	err = clSetKernelArg(filterKernel, 1, sizeof(cl_mem), &filterBuffer);
	checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

	size_t globalWorkSize[2] = {height, (width + 4)/4}; // +4 is for padding (as defined in SignalProcessor constructor) because the last complex number overlaps to this area.
	err = clEnqueueNDRangeKernel(queue, filterKernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueNDRangeKernel()");

	// IFFT.
	errFFT = clfftEnqueueTransform(ifftPlanBatch, CLFFT_BACKWARD, 1, &queue, 0, nullptr, nullptr, &outBuffer, nullptr, nullptr);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftEnqueueTransform");
}

