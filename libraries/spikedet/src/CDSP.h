#ifndef CDSP_H
#define CDSP_H

#include "wx/wx.h"
#include "wx/thread.h"

#include <algorithm>
#include <exception>
#include <vector>
#include <complex>
#include <cmath>
#include <string.h>
#include <cstdlib>

#include "CException.h"
#include "Definitions.h"

/// long double PI value specifications
#define PId	3.141592653589793238462643383279502884L
/// float PI value specifications
#define PIf	3.1415926535897932384626433832795f

/// Data type specifications - FLOAT
#define RTF_FLOAT	0
/// Data type specifications - DOUBLE
#define RTF_DOUBLE	1

/**
 * Definition of the type of filter for Butterworth filter design.
 */
enum FILTERTYPE
{
	/// Flag of LOWPASS filter type
	LOWPASS = 0,
	/// Flag of HIGHPASS filter type
	HIGHPASS = 1
};

/**
 * Coefficients of filter design
 */
struct coeff {
	/// A numerator
	double* num;
	/// A denumerator
	double* den;
};

/**
 * Static class for digital signal processing.
 */
class CDSP
{
	// methods
public:
	/**
	 * Method for digital signal resampling - In this program is used for decimating.
	 * The result is save in the data param.
	 * @param data array of wxVectors representing input / output data
	 * @param countChannels count channels in input signal (data) - size of the array
	 * @param actualFS actual sample rate
	 * @param requiredFS required sample rate
	 */
	static void Resample(wxVector<SIGNALTYPE>*& data, const int& countChannels, const int& actualFS, const int& requiredFS, bool original);

	/**
	 * Digital signal filtering 10-60Hz
	 * @param data wxVector array with input / output data
	 * @param countChannels count channels in input signal (data) - size of the array
	 * @param fs sampling rate of input signal
	 * @param bandwidth BANDWIDTH struct containing integers band_low and band_high.
	 */
	static void Filtering(wxVector<SIGNALTYPE>* data, const int& countChannels, const int& fs, const BANDWIDTH& bandwidth);

	/**
	 * Digital signal filtering Nx50hz
	 * @param data Array of wxVector containing input / output data.
	 * @param countChannels count channels in input signal (data) - size of the array
	 * @param fs sampling rate of input signal
	 * @param hum_fs Integer main_hum_freq from spike detector setting.
	 * @param bandwidth BANDWIDTH struct containing integers band_low and band_high.
	 */
	static void Filt50Hz(wxVector<SIGNALTYPE>* data, const int& countChannels, const int& fs, const int& hum_fs, const BANDWIDTH& bandwidth);

	/**
	 * Zero-phase forward and reverse digital IIR filtering.
	 * This function is similar as Matlab function: y = filtfilt(b,a,x).
	 * @param A vector describing filter.
	 * @param B vector describing filter.
	 * @param X input and output vector.
	 */
	static void FiltFilt(const wxVector<double>& B, const wxVector<double>& A, wxVector<SIGNALTYPE>* X);

	/**
	 * Calculation of the absolute values of the Hilbert transform.
	 * Performs a Hilbert transform and then calculates its absolute value.
	 * This function is based on the MATLAB function hilbert(Xr).
	 * @param data Input and output vector containing data.
	 */
	static void AbsHilbert(wxVector<SIGNALTYPE>& data);

	/**
	 * Butterworth filter order selection.
	 * Calculating the order N of the lowest order digital Butterworth filter which has a passband ripple of no more than Rp dB
	 * and a stopband attenuation of at least Rs dB. Also calculate Wn, the Butterworth natural frequency (or, the "3 dB frequency")
	 * to use with BUTTER to achieve the specifications. This function is based on Matlab function: [order,wn] = buttord(wp,ws,rp,rs,opt).
	 * @param wp the passband edge frequency, normalized from 0 to 1
	 * @param ws input and output variable representing stopband edge frequencies, replace output Wn, normalized from 0 to 1
	 * @param rp a passband ripple max value dB
	 * @param rs a stopband attenuation dB
	 * @param order ouput variable to which it is stored the order
	 */
	static void Buttord(const double& wp, double& ws, const double& rp, const double& rs, int& order);

	/**
	 * Designs an Nth order lowpass digital Butterworth filter and returns the filter coefficients in length N+1 vectors B (numerator) and A (denominator).
	 * This function is based on Matlab function: [num, den, z, p] = butter(n, Wn, varargin).
	 * @param b ouput vector in which are saved numerator values
	 * @param a output vector in which are saved denominator values
	 * @param order filter order
	 * @param Wn the cutoff frequency, must be 0.0 < Wn < 1.0, with 1.0 corresponding to half the sample rate.
	 * @param ftype filter type: HIGHPASS or LOWPASS, difine on FILTERTYPE
	 */
	static bool Butter(wxVector<double>& b, wxVector<double>& a, const int& order, const double& Wn, FILTERTYPE ftype);

private:
	static void filt50Hz(wxVector<SIGNALTYPE>* data, const int& countChannels, const wxVector<double>& B, const wxVector<double>& A);

	/**
	 * This method is based on functions from librtfilters \ref http://cnbi.epfl.ch/software/rtfilter.html
	 */
	static void getPoleCoefs(double p, double np, double fc, double r, int highpass, double a[3], double b[3]);

	/**
	 * Compute Chebyshev coefficients. This method is based on functions from librtfilters \ref http://cnbi.epfl.ch/software/rtfilter.html
	 */
	static int computeChebyIir(double *num, double *den, unsigned int num_pole,
							   int highpass, double ripple, double cutoff_freq);

	/**
	 * Calculate coefficients for Butterworth filter.
	 * @param coeff contains calculated coefficients.
	 * @return 0 on success or negative value on failure.
	 */
	static int calcButterCoeff(unsigned int nchann, int proctype, double fc,
							   unsigned int num_pole, int highpass, struct coeff *coeff);
};


// auxiliary classes

/**
 * Class representing thread-resampling one channel
 */
class CResamplingThread : public wxThread
{
	// methods
public:
	/**
	 * A constructor.
	 * @param data vector with input data, output will be stored in it
	 * @param actFS actual sample rate
	 * @param requiredFS required sample rate
	 */
	CResamplingThread(wxVector<SIGNALTYPE>* data, const int& actFS, const int& requiredFS, bool original);

	/**
	 * A virtual desctructor.
	 */
	virtual ~CResamplingThread();

	/**
	 * This is the entry point of the thread.
	 * @return ExitCode
	 */
	virtual ExitCode Entry();
private:

	// variables
public:
private:
	/// actual sample rate
	int 			  	  m_actFS;
	/// required sample rate
	int 			 	  m_requiredFS;
	/// input / output data
	wxVector<SIGNALTYPE>* m_data;
	bool m_original;
};

/**
 * Class representing thread-filtering one channel
 * It implement function filtfilt from MATLAB.
 * There is used filter from: <a href="http://stackoverflow.com/questions/17675053/matlabs-filtfilt-algorithm/27270420#27270420">stackoverflow</a>
 */
class CFiltFilt : public wxThread
{
	// methods
public:
	/**
	 * A constructor.
	 * @param B the numerator coefficients
	 * @param A the denominator coefficients
	 * @param X data for filtering
	 */
	CFiltFilt(const wxVector<double>& B, const wxVector<double>& A, wxVector<SIGNALTYPE>* X);

	/**
	 * A desctructor.
	 */
	virtual ~CFiltFilt();

	/**
	 * This is the entry point of the thread.
	 * @return ExitCode
	 */
	virtual ExitCode Entry();
private:
	// methonds from: http://stackoverflow.com/questions/17675053/matlabs-filtfilt-algorithm/27270420#27270420
	void filtFilt(std::vector<double> B, std::vector<double> A, const std::vector<double> &X, std::vector<double> &Y);
	void add_index_range(std::vector<int> &indices, int beg, int end, int inc);
	void add_index_const(std::vector<int> &indices, int value, size_t numel);
	void append_vector(std::vector<double> &vec, const std::vector<double> &tail);
	std::vector<double> subvector_reverse(const std::vector<double> &vec, int idx_end, int idx_start);
	inline int max_val(const std::vector<int>& vec){ return std::max_element(vec.begin(), vec.end())[0]; }
	void filter(std::vector<double> B, std::vector<double> A, const std::vector<double> &X, std::vector<double> &Y, std::vector<double> &Zi);

	// variables
public:
private:
	/// the numerator coefficients
	wxVector<double>      m_B;
	/// the denominator coefficients
	wxVector<double>      m_A;
	/// data for filtering
	wxVector<SIGNALTYPE>* m_X;
};

#endif
