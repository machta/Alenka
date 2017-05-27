#include <gtest/gtest.h>

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaSignal/montageprocessor.h>

#include <functional>

using namespace std;
using namespace AlenkaSignal;

namespace
{

void compareFloat(float a, float b)
{
	EXPECT_FLOAT_EQ(a, b);
}

void compareDouble(double a, double b)
{
	EXPECT_DOUBLE_EQ(a, b);
}

template<class T>
bool testMontage(const string& src, OpenCLContext* context)
{
	string msg;
	bool res = Montage<T>::test(src, context, &msg);

	if (!res)
		cerr << msg << endl;

	return res;
}

template<class T>
void test(function<void(T, T)> compare, int outputCopies)
{
	int n = 20;
	int inChannels = 3;
	int offset = 5;
	cl_int err;

	OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);
	MontageProcessor<T> processor(n, inChannels, outputCopies);

	cl_command_queue queue = clCreateCommandQueue(context.getCLContext(), context.getCLDevice(), 0, &err);
	checkClErrorCode(err, "clCreateCommandQueue");

	string src = "out = in(0);";
	ASSERT_TRUE(testMontage<T>(src, &context));
	Montage<T> m1(src, &context);

	src = "out = in(1);";
	ASSERT_TRUE(testMontage<T>(src, &context));
	Montage<T> m2(src, &context);

	src = "out = in(0) + in(1);";
	ASSERT_TRUE(testMontage<T>(src, &context));
	Montage<T> m3(src, &context);

	src = "out = in(2)*3.14;";
	ASSERT_TRUE(testMontage<T>(src, &context));
	Montage<T> m4(src, &context);

	src = "out = -1;";
	ASSERT_TRUE(testMontage<T>(src, &context));
	Montage<T> m5(src, &context);

	vector<Montage<T>*> montage = {&m1, &m2, &m3, &m4, &m5};
	vector<T> signal;

	for (int j = 0; j < inChannels; j++)
		for (int i = 1; i <= n; i++)
			signal.push_back(10*pow(10, j) + i);

	vector<T> output(outputCopies*(n - offset)*montage.size());

	cl_mem_flags flags = CL_MEM_READ_WRITE;
	size_t outBufferSize = outputCopies*(n - offset)*montage.size()*sizeof(T);

	cl_mem inBuffer = clCreateBuffer(context.getCLContext(), flags | CL_MEM_COPY_HOST_PTR, n*inChannels*sizeof(T), signal.data(), &err);
	checkClErrorCode(err, "clCreateBuffer");

	cl_mem outBuffer = clCreateBuffer(context.getCLContext(), flags, outBufferSize, nullptr, &err);
	checkClErrorCode(err, "clCreateBuffer");

	processor.process(montage, inBuffer, outBuffer, queue, n - offset);

	err = clEnqueueReadBuffer(queue, outBuffer, CL_TRUE, 0, outBufferSize, output.data(), 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueReadBuffer");

	err = clReleaseCommandQueue(queue);
	checkClErrorCode(err, "clReleaseCommandQueue");

	err = clReleaseMemObject(inBuffer);
	checkClErrorCode(err, "clReleaseMemObject");

	err = clReleaseMemObject(outBuffer);
	checkClErrorCode(err, "clReleaseMemObject");

	for (int i = 0; i < n - offset; ++i)
	{
		for (int j = 0; j < outputCopies; ++j)
		{
			compare(output[outputCopies*((n - offset)*0 + i) + j], signal[i]);
			compare(output[outputCopies*((n - offset)*1 + i) + j], signal[n + i]);
			compare(output[outputCopies*((n - offset)*2 + i) + j], signal[i] + signal[n + i]);
			compare(output[outputCopies*((n - offset)*3 + i) + j], signal[2*n + i]*3.14);
			compare(output[outputCopies*((n - offset)*4 + i) + j], -1);
		}
	}
}

} // namespace

TEST(simple_montage_test, float_1)
{
	test<float>(&compareFloat, 1);
}

TEST(simple_montage_test, double_1)
{
	test<double>(&compareDouble, 1);
}

TEST(simple_montage_test, float_2)
{
	test<float>(&compareFloat, 2);
}

TEST(simple_montage_test, double_2)
{
	test<double>(&compareDouble, 2);
}

TEST(simple_montage_test, float_n)
{
	for (int i = 3; i < 10; ++i)
		test<float>(&compareFloat, i);
}

TEST(simple_montage_test, double_n)
{
	for (int i = 3; i < 10; ++i)
		test<double>(&compareDouble, i);
}
