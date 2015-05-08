#ifndef FILTER_H
#define FILTER_H

#include "../openclcontext.h"

#include <cstdio>
#include <cmath>
#include <vector>

#include <CL/cl_gl.h>
#include <clFFT.h>

/**
 * @brief A class for computing FIR filter coefficients.
 *
 * After construction the filter has no effect, i.e. it is an all-pass filter.
 *
 * The filter can be configured to work as any combination of the following
 * basic filter types:
 * * low-pass,
 * * high-pass and
 * * notch filter.
 *
 * The filter can be configured by the appropriate set functions.
 *
 * The coefficients are computed using the frequency-sampling method.
 */
class Filter
{
public:
	/**
	 * @brief Filter constructor.
	 * @param M The length of the filter and the number of coefficients that
	 * will be returned.
	 * @param Fs The sampling frequency.
	 */
	Filter(unsigned int M, double Fs);
	~Filter();

	/**
	 * @brief Print coefficients to the file stream.
	 */
	void printCoefficients(FILE* file);

	/**
	 * @brief Returns a vector with the coefficients.
	 */
	std::vector<double> computeCoefficients();

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

	/**
	 * @brief If this value is passed to setLowpass, low-pass has no effect.
	 */
	static const double LOWPASS_OFF_VALUE;

	/**
	 * @brief If this value is passed to setHighpass(), high-pass has no effect.
	 */
	static const double HIGHPASS_OFF_VALUE;

private:
	unsigned int M;
	double Fs;
	double lowpass;
	double highpass;
	bool notch;

	OpenCLContext context;
	cl_command_queue queue;
	clfftPlanHandle plan;
	double* coefficients;
	double notchF;

	double hammingWindow(int n, int M)
	{
		using namespace std;
		const double tmp = 2*M_PI*n/(M - 1);
		return 0.54 - 0.46*cos(tmp);
	}
	double blackmanWindow(int n, int M)
	{
		using namespace std;
		const double a = 0.16, a0 = (1 - a)/2, a1 = 0.5, a2 = a/2, tmp = 2*M_PI*n/(M - 1);
		return a0 - a1*cos(tmp) + a2*cos(2*tmp);
	}
};

#endif // FILTER_H
