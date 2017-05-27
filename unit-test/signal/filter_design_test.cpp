#include <gtest/gtest.h>

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filter.h>
#include <AlenkaSignal/filterprocessor.h>

#include <functional>

using namespace std;
using namespace AlenkaSignal;

namespace
{

double answerLowpassD[8] = {-0.105969883127822, 0.0293291419087276, 0.220670858091272, 0.355969883127822, 0.355969883127822, 0.220670858091272, 0.0293291419087275, -0.105969883127822};
float answerLowpassF[8]  = {-0.105969883127822, 0.0293291419087276, 0.220670858091272, 0.355969883127822, 0.355969883127822, 0.220670858091272, 0.0293291419087275, -0.105969883127822};

void compareFloat(float a, float b)
{
	//EXPECT_FLOAT_EQ(a, b);
	EXPECT_NEAR(a, b, 0.0000001);
}

void compareDouble(double a, double b)
{
	//EXPECT_DOUBLE_EQ(a, b);
	EXPECT_NEAR(a, b, 0.000000001);
}

template<class T>
void test(function<void(T, T)> compare, T* answer)
{
	OpenCLContext::clfftInit();

	{
		int n = 20;
		cl_int err;

		OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);
		FilterProcessor<T> processor(n, 1, &context);

		vector<T> signal;
		vector<T> output(n);
		for (int i = 1; i <= n; i++)
			signal.push_back(i);

		cl_command_queue queue = clCreateCommandQueue(context.getCLContext(), context.getCLDevice(), 0, &err);
		checkClErrorCode(err, "clCreateCommandQueue");

		cl_mem_flags flags = CL_MEM_READ_WRITE;

		cl_mem inBuffer = clCreateBuffer(context.getCLContext(), flags | CL_MEM_COPY_HOST_PTR, (n + 2)*sizeof(T), signal.data(), &err);
		checkClErrorCode(err, "clCreateBuffer");

		cl_mem outBuffer = clCreateBuffer(context.getCLContext(), flags, (n + 2)*sizeof(T), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		Filter<T> filter(8, 200);
		filter.setLowpassOn(true);
		filter.setLowpass(50);
		processor.changeSampleFilter(8, filter.computeSamples());

		processor.process(inBuffer, outBuffer, queue);

		err = clEnqueueReadBuffer(queue, outBuffer, CL_TRUE, 0, n*sizeof(T), output.data(), 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReadBuffer");

		auto res = processor.getCoefficients();
		for (int i = 0; i < 8; ++i)
			compare(res[i], answer[i]);

		err = clReleaseMemObject(inBuffer);
		checkClErrorCode(err, "clReleaseMemObject");

		err = clReleaseMemObject(outBuffer);
		checkClErrorCode(err, "clReleaseMemObject");
	}

	OpenCLContext::clfftDeinit();
}

} // namespace

TEST(filter_design_test, simple_float)
{
	test<float>(&compareFloat, answerLowpassF);
}

TEST(filter_design_test, simple_double)
{
	test<double>(&compareDouble, answerLowpassD);
}
