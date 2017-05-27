#include <gtest/gtest.h>

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filter.h>
#include <AlenkaSignal/filterprocessor.h>

#include <algorithm>
#include <functional>
#include <cmath>

using namespace std;
using namespace AlenkaSignal;

namespace
{

template<class T>
void testFilter(Filter<T> filter, int M, int channelCount, const vector<T>& data, const vector<T>& answer, double relativeError = 0.0001)
{
	OpenCLContext::clfftInit();

	{
		cl_int err;
		int n = data.size()/channelCount + M - 1;

		OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);
		FilterProcessor<T> processor(n, channelCount, &context);

		processor.changeSampleFilter(M, filter.computeSamples());

		vector<T> output(n*channelCount);

		vector<T> input(n*channelCount, 0);
		for (int j = 0; j < channelCount; j++)
			for (int i = 0; i < data.size()/channelCount; i++)
				input[j*n + i + M - 1] = data[j*data.size()/channelCount + i];

		cl_command_queue queue = clCreateCommandQueue(context.getCLContext(), context.getCLDevice(), 0, &err);
		checkClErrorCode(err, "clCreateCommandQueue");

		cl_mem_flags flags = CL_MEM_READ_WRITE;

		cl_mem inBuffer = clCreateBuffer(context.getCLContext(), flags | CL_MEM_COPY_HOST_PTR, (n + 2)*channelCount*sizeof(T), input.data(), &err);
		checkClErrorCode(err, "clCreateBuffer");

		cl_mem outBuffer = clCreateBuffer(context.getCLContext(), flags, (n + 2)*channelCount*sizeof(T), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		processor.process(inBuffer, outBuffer, queue);

		err = clEnqueueReadBuffer(queue, outBuffer, CL_TRUE, 0, n*channelCount*sizeof(T), output.data(), 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReadBuffer");

		double maxError = 0;
		for (int j = 0; j < channelCount; j++)
		{
			for (int i = 0; i < answer.size()/channelCount - processor.delaySamples(); i++)
			{
				T a = output[j*n + i + processor.discardSamples() + processor.delaySamples()];
				T b = answer[j*answer.size()/channelCount + i];
				maxError = max<double>(maxError, abs(a - b));
			}
		}

		ASSERT_LT(maxError, relativeError);

		err = clReleaseCommandQueue(queue);
		checkClErrorCode(err, "clReleaseCommandQueue");

		err = clReleaseMemObject(inBuffer);
		checkClErrorCode(err, "clReleaseMemObject");

		err = clReleaseMemObject(outBuffer);
		checkClErrorCode(err, "clReleaseMemObject");
	}

	OpenCLContext::clfftDeinit();
}

template<class T>
void generateSin(double A, double f, double Fs, int shift, int channelIndex, int length, int channelCount, vector<T>* data)
{
	for (int i = 0; i < length; i++)
		data->at(channelIndex*length + i) = A*sin((i + shift)*f/Fs*2*M_PI);
}

template<class T>
vector<T> addSignal(int length, int channelCount, vector<T>* data1, vector<T>* data2)
{
	vector<T> tmp;
	for (int i = 0; i < length*channelCount; i++)
		tmp.push_back(data1->at(i) + data2->at(i));
	return tmp;
}

} // namespace

TEST(filter_test, allpass_float)
{
	int n = 200*2 + 1;
	int c = 2;

	vector<float> data(n*c);
	for (int i = 0; i < c; i++)
		generateSin(i + 1, 2, 200, i*10, i, n, c, &data);

	Filter<float> filter(200, 200);

	//testFilter(filter, 200, c, data, data); // TODO: Turn this on.
}
