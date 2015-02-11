#ifndef FILTER_H
#define FILTER_H

#include "../openclcontext.h"

#include <cstdio>
#include <cmath>

#include <CL/cl.h>
#include <clFFT.h>

class Filter
{
public:
	Filter(unsigned int M, double Fs);
	~Filter();

	double* computeCoefficients();
	void printCoefficients(FILE* file);

    double getLowpass() const
	{
		return lowpass*Fs/2;
	}
    void setLowpass(double value)
	{
		lowpass = value/Fs*2;
	}
    double getHighpass() const
	{
		return highpass*Fs/2;
	}
    void setHighpass(double value)
	{
		highpass = value/Fs*2;
	}
    bool getNotch() const
	{
		return notch;
	}
    void setNotch(bool value)
	{
		notch = value;
	}

private:
	unsigned int M;
	double Fs;
	double lowpass;
	double highpass;
	bool notch;

    OpenCLContext clContext;
	cl_command_queue queue;
	clfftPlanHandle plan;
	double* coefficients;
	double notchF;

	double hammingWindow(int n, int M)
	{
		using namespace std;
		return 0.54 - 0.46*cos(2*M_PI*n/(M - 1));
	}
	double blackmanWindow(int n, int M)
	{
		using namespace std;
		const double a = 0.16, a0 = (1 - a)/2, a1 = 0.5, a2 = a/2, tmp = 2*M_PI*n/(M - 1);
		return a0 - a1*cos(tmp) + a2*cos(2*tmp);
	}
};

#endif // FILTER_H
