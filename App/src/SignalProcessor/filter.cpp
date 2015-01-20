#include "filter.h"

#include "../options.h"

#include <sstream>
#include <stdexcept>
#include <complex>

using namespace std;

Filter::Filter(unsigned int M, double Fs) : M(M), Fs(Fs), lowpass(2),
	highpass(-1), notch(false), notchF(50/Fs*2),
	context(PROGRAM_OPTIONS->get("platform").as<int>(), 0, CL_DEVICE_TYPE_CPU)
{
	size_t size = M;
	clfftStatus errFFT = clfftCreateDefaultPlan(&plan, context.getCLContext(), CLFFT_1D, &size);
	if (errFFT != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clfftCreateDefaultPlan returned with error code " << errFFT << ".";
		throw runtime_error(ss.str());
	}

	clfftSetPlanPrecision(plan, CLFFT_DOUBLE);
	clfftSetLayout(plan, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
	clfftSetResultLocation(plan, CLFFT_INPLACE);

	cl_int errCL;
	queue = clCreateCommandQueue(context.getCLContext(), context.getCLDevice(), 0, &errCL);
	if (errCL != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clCreateCommandQueue returned with error code " << errCL << ".";
		throw runtime_error(ss.str());
	}

	clfftBakePlan(plan, 1, &queue, nullptr, nullptr);

	coefficients = new double[2*(1 + M/2)];
}

Filter::~Filter()
{
	clReleaseCommandQueue(queue);
	clfftDestroyPlan(&plan);
	delete[] coefficients;
}

double* Filter::computeCoefficients()
{
	int cM = 1 + M/2;

	// Initialize cofficients with the values of Hr.
	for (int i = 0; i < cM; ++i)
	{
		double f = 2.*i/M;
		double val = 1;

		if (f >= lowpass)
		{
			val = 0;
		}
		else if (f <= highpass)
		{
			val = 0;
		}
		else if (notch)
		{
			double tmp = round(f/notchF);
			tmp = fabs(f - tmp*notchF);
			if (tmp <= 1./M*Fs/M)
			{
				val = 0;
			}
		}

		coefficients[2*i] = val;
		coefficients[2*i + 1] = 0;
	}

	// Multiply Hr by exp(...) to make the frequency response H. (eq. 10.2.35)
	for (int i = 0; i < cM; ++i)
	{
		complex<double> tmp(0, 1);
		tmp *= -2*M_PI*i*(M - 1)/2/M;
		tmp = exp(tmp);

		complex<double> tmp2(coefficients[2*i], coefficients[2*i + 1]);
		tmp *= tmp2;

		coefficients[2*i] = tmp.real();
		coefficients[2*i + 1] = tmp.imag();
	}

	// Compute the iFFT of H to make the FIR filter coefficients h. (eq. 10.2.33)
	cl_int errCL;
	cl_mem buffer = clCreateBuffer(context.getCLContext(), CL_MEM_USE_HOST_PTR, 2*cM*sizeof(double), coefficients, &errCL);
	if (errCL != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clCreateBuffer returned with error code " << errCL << ".";
		throw runtime_error(ss.str());
	}

	clfftStatus errFFT = clfftEnqueueTransform(plan, CLFFT_BACKWARD, 1, &queue, 0, nullptr, nullptr, &buffer, nullptr, nullptr);
	if (errFFT != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clfftEnqueueTransform returned with error code " << errFFT << ".";
		throw runtime_error(ss.str());
	}

	clFinish(queue);
	//clEnqueueReadBuffer(queue, buffer, true, 0, M*sizeof(double), coefficients, 0, nullptr, nullptr);

	// Try to improve filter characteristics by applying a window function.
	string window = PROGRAM_OPTIONS->get("window").as<string>();

	for (int i = 0; i < M; ++i)
	{
		if (window == "hamming")
		{
			coefficients[i] *= hammingWindow(i, M);
		}
		else if (window == "blackman")
		{
			coefficients[i] *= blackmanWindow(i, M);
		}
	}

	clReleaseMemObject(buffer);
	return coefficients;
}

void Filter::printCoefficients(FILE* file)
{
	fprintf(file, "%lf\n%lf\n%lf\n", Fs, getLowpass(), getHighpass());
	for (int i = 0; i < M; ++i)
	{
		fprintf(file, "%lf\n", coefficients[i]);
	}
}





