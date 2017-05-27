#include "../include/AlenkaSignal/filterprocessor.h"

#include "../include/AlenkaSignal/openclcontext.h"
#include "../include/AlenkaSignal/openclprogram.h"

#include <fasttransforms.h>
#include <clFFT.h>

#include <cmath>
#include <complex>
#include <type_traits>

using namespace std;

namespace
{

// Defines const char* KERNELS_SOURCE.
#include "kernels.cl"

template<class T>
T hammingWindow(int n, int M)
{
	const T tmp = 2*M_PI*n/(M - 1);
	return 0.54 - 0.46*cos(tmp);
}

template<class T>
T blackmanWindow(int n, int M)
{
	const T a = 0.16, a0 = (1 - a)/2, a1 = 0.5, a2 = a/2, tmp = 2*M_PI*n/(M - 1);
	return a0 - a1*cos(tmp) + a2*cos(2*tmp);
}

string clfftErrorCodeToString(clfftStatus code)
{
#define CASE(a_) case a_: return #a_
	switch (code)
	{
		CASE(CLFFT_BUGCHECK);
		CASE(CLFFT_NOTIMPLEMENTED);
		CASE(CLFFT_TRANSPOSED_NOTIMPLEMENTED);
		CASE(CLFFT_FILE_NOT_FOUND);
		CASE(CLFFT_FILE_CREATE_FAILURE);
		CASE(CLFFT_VERSION_MISMATCH);
		CASE(CLFFT_INVALID_PLAN);
		CASE(CLFFT_DEVICE_NO_DOUBLE);
		CASE(CLFFT_DEVICE_MISMATCH);
		CASE(CLFFT_ENDSTATUS);
	default:
		return AlenkaSignal::OpenCLContext::clErrorCodeToString(code);
	}
#undef CASE
}

void CFCEC(clfftStatus val, string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Unexpected error code: " << clfftErrorCodeToString(val);
	ss << ", required CLFFT_SUCCESS. ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

/**
 * @brief Simplified error code test for clFFT functions
 * @param val_ The error code.
 */
#define checkClfftErrorCode(val_, message_) if((val_) != CLFFT_SUCCESS) { std::stringstream ss; ss << message_; CFCEC(val_, ss.str(), __FILE__, __LINE__); }

} // namespace

namespace AlenkaSignal
{

template<class T>
FilterProcessor<T>::FilterProcessor(unsigned int blockLength, unsigned int channels, OpenCLContext* context)
	: blockLength(blockLength), blockChannels(channels)
{
	assert(blockLength%2 == 0);

	cl_int err;
	clfftStatus errFFT;

	clfftPrecision precision = CLFFT_SINGLE;
	string kernelsSource;

	if (is_same<double, T>::value)
	{
		precision = CLFFT_DOUBLE;
		kernelsSource = "#define float double\n#define float2 double2\n\n";
	}

	kernelsSource += KERNELS_SOURCE;
	OpenCLProgram program(kernelsSource, context);

	filterKernel = program.createKernel("filter");
	zeroKernel = program.createKernel("zero");

	cl_mem_flags flags = CL_MEM_READ_WRITE;
	filterBuffer = clCreateBuffer(context->getCLContext(), flags, (blockLength + 2)*sizeof(T), nullptr, &err);
	checkClErrorCode(err, "clCreateBuffer");

	// Construct the fft plans.
	size_t size = blockLength;
	size_t bufferDistance = size + 2;

	errFFT = clfftCreateDefaultPlan(&fftPlan, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	errFFT = clfftSetPlanPrecision(fftPlan, precision);
	checkClfftErrorCode(errFFT, "clfftSetPlanPrecision()");
	errFFT = clfftSetLayout(fftPlan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	checkClfftErrorCode(errFFT, "clfftSetLayout()");
	errFFT = clfftSetResultLocation(fftPlan, CLFFT_INPLACE);
	checkClfftErrorCode(errFFT, "clfftSetResultLocation()");
	errFFT = clfftSetPlanBatchSize(fftPlan, 1);
	checkClfftErrorCode(errFFT, "clfftSetPlanBatchSize()");
	//clfftSetPlanDistance(fftPlan, bufferDistance, bufferDistance/2);

	errFFT = clfftCreateDefaultPlan(&fftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	errFFT = clfftSetPlanPrecision(fftPlanBatch, precision);
	checkClfftErrorCode(errFFT, "clfftSetPlanPrecision()");
	errFFT = clfftSetLayout(fftPlanBatch, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	checkClfftErrorCode(errFFT, "clfftSetLayout()");
	errFFT = clfftSetResultLocation(fftPlanBatch, CLFFT_OUTOFPLACE);
	checkClfftErrorCode(errFFT, "clfftSetResultLocation()");
	errFFT = clfftSetPlanBatchSize(fftPlanBatch, blockChannels);
	checkClfftErrorCode(errFFT, "clfftSetPlanBatchSize()");
	errFFT = clfftSetPlanDistance(fftPlanBatch, bufferDistance, bufferDistance/2);
	checkClfftErrorCode(errFFT, "clfftSetPlanDistance()");

	errFFT = clfftCreateDefaultPlan(&ifftPlanBatch, context->getCLContext(), CLFFT_1D, &size);
	checkClfftErrorCode(errFFT, "clfftCreateDefaultPlan()");
	errFFT = clfftSetPlanPrecision(ifftPlanBatch, precision);
	checkClfftErrorCode(errFFT, "clfftSetPlanPrecision()");
	errFFT = clfftSetLayout(ifftPlanBatch, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	checkClfftErrorCode(errFFT, "clfftSetLayout()");
	errFFT = clfftSetResultLocation(ifftPlanBatch, CLFFT_INPLACE);
	checkClfftErrorCode(errFFT, "clfftSetResultLocation()");
	errFFT = clfftSetPlanBatchSize(ifftPlanBatch, blockChannels);
	checkClfftErrorCode(errFFT, "clfftSetPlanBatchSize()");
	errFFT = clfftSetPlanDistance(ifftPlanBatch, bufferDistance/2, bufferDistance);
	checkClfftErrorCode(errFFT, "clfftSetPlanDistance()");
}

template<class T>
FilterProcessor<T>::~FilterProcessor()
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

// This version preserves input. TODO: Make another "destructive" version that uses outBuffer as a temporary buffer
// and copies the result back to inBuffer. This new vesion could then be used with multiple parallel queues at once.
// This would also require to handle filterBuffer differently: it must be updated before process is used
// and not wait until it is used for the first time. Then it could be too late as, while it is updating,
// another parallel queue will already need the buffer.

template<class T>
void FilterProcessor<T>::process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue)
{
	assert(inBuffer != outBuffer);

	cl_int err;
	clfftStatus errFFT;
	size_t inSize, outSize, minSize = (blockLength + 2)*blockChannels*sizeof(T);

	err = clGetMemObjectInfo(inBuffer, CL_MEM_SIZE, sizeof(size_t), &inSize, nullptr);
	checkClErrorCode(err, "clGetMemObjectInfo");

	if (inSize < minSize)
		throw runtime_error("FilterProcessor: the inBuffer is too small.");

	err = clGetMemObjectInfo(outBuffer, CL_MEM_SIZE, sizeof(size_t), &outSize, nullptr);
	checkClErrorCode(err, "clGetMemObjectInfo");

	if (outSize < minSize)
		throw runtime_error("FilterProcessor: the outBuffer is too small.");

	if (coefficientsChanged)
	{
		err = clEnqueueWriteBuffer(queue, filterBuffer, CL_TRUE, 0, M*sizeof(T), coefficients.data(), 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueWriteBuffer()");

		//printBuffer("before_filterBuffer.txt", filterBuffer, queue);

		err = clSetKernelArg(zeroKernel, 0, sizeof(cl_mem), &filterBuffer);
		checkClErrorCode(err, "clSetKernelArg()");

		size_t globalWorkOffset = M;
		size_t globalWorkSize = blockLength + 2 - globalWorkOffset;
		err = clEnqueueNDRangeKernel(queue, zeroKernel, 1, &globalWorkOffset, &globalWorkSize, nullptr, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueNDRangeKernel()");

		//printBuffer("after_filterBuffer_zero.txt", filterBuffer, queue);

		errFFT = clfftEnqueueTransform(fftPlan, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &filterBuffer, nullptr, nullptr);
		checkClfftErrorCode(errFFT, "clfftEnqueueTransform");

		//printBuffer("after_filterBuffer.txt", filterBuffer, queue);
	}

	//OpenCLContext::printBuffer("before_fft.txt", inBuffer, queue);

	// FFT.
	errFFT = clfftEnqueueTransform(fftPlanBatch, CLFFT_FORWARD, 1, &queue, 0, nullptr, nullptr, &inBuffer, &outBuffer, nullptr);
	checkClfftErrorCode(errFFT, "clfftEnqueueTransform");

	//OpenCLContext::printBuffer("after_fft.txt", outBuffer, queue);

	// Multiply.
	err = clSetKernelArg(filterKernel, 0, sizeof(cl_mem), &outBuffer);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(filterKernel, 1, sizeof(cl_mem), &filterBuffer);
	checkClErrorCode(err, "clSetKernelArg()");

	size_t globalWorkSize[2] = {blockLength/2 + 1, blockChannels};

	err = clEnqueueNDRangeKernel(queue, filterKernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueNDRangeKernel()");

	//OpenCLContext::printBuffer("after_multiply.txt", outBuffer, queue);

	// IFFT.
	errFFT = clfftEnqueueTransform(ifftPlanBatch, CLFFT_BACKWARD, 1, &queue, 0, nullptr, nullptr, &outBuffer, nullptr, nullptr);
	checkClfftErrorCode(errFFT, "clfftEnqueueTransform");

	//OpenCLContext::printBuffer("after_ifft.txt", outBuffer, queue);
}

template<class T>
void FilterProcessor<T>::changeSampleFilter(int M, const std::vector<T>& samples)
{
	assert((int)samples.size() == (M + 1)/2 && "Assure the right number of samples was provided.");

	coefficientsChanged = true;
	this->M = M;

	int cM = 1 + M/2;

	alglib::complex_1d_array inArray;
	inArray.setlength(cM);
	inArray[cM - 1].x = 0;
	inArray[cM - 1].y = 0;

	for (unsigned int i = 0; i < /*cM*/samples.size(); ++i)
	{
		assert(i < /*(int)*/samples.size());

		inArray[i].x = samples[i];
		inArray[i].y = 0;
	}

	// Multiply Hr by exp(...) to make the frequency response H. (eq. 10.2.35)
	for (int i = 0; i < cM; ++i)
	{
		/*complex<T> tmp(0, 1);
		tmp *= -2*M_PI*i*(M - 1)/2/M;
		tmp = exp(tmp);

		complex<T> tmp2(coefficients[2*i], coefficients[2*i + 1]);
		tmp *= tmp2;

		coefficients[2*i] = tmp.real();
		coefficients[2*i + 1] = tmp.imag();*/

		complex<double> tmp(0, 1);
		tmp *= -2*M_PI*i*(M - 1)/2/M;
		tmp = exp(tmp);

		inArray[i] *= alglib::complex(tmp.real(), tmp.imag());
	}

	// Compute the iFFT of H to make the FIR filter coefficients h. (eq. 10.2.33)
	alglib::real_1d_array outArray;
	outArray.setlength(M);
	alglib::fftr1dinv(inArray, M, outArray);

	coefficients.resize(M);
	for (int i = 0; i < M; i++)
		coefficients[i] = outArray[i];
}

template<class T>
void FilterProcessor<T>::applyWindow(WindowFunction windowFunction)
{
	// TODO: Perhaps apply the window function on the GPU as an optimization.
	assert(static_cast<int>(coefficients.size()) == M);
	coefficientsChanged = true;

	if (windowFunction == WindowFunction::Hamming)
	{
		for (int i = 0; i < M; ++i)
			coefficients[i] *= hammingWindow<T>(i, M);
	}
	else if (windowFunction == WindowFunction::Blackman)
	{
		for (int i = 0; i < M; ++i)
			coefficients[i] *= blackmanWindow<T>(i, M);
	}
}

template class FilterProcessor<float>;
template class FilterProcessor<double>;

} // namespace AlenkaSignal
