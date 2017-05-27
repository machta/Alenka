#ifndef ALENKASIGNAL_FILTER_H
#define ALENKASIGNAL_FILTER_H

#include <cmath>
#include <vector>
#include <cstdio>

namespace AlenkaSignal
{

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
template<class T>
class Filter
{
	unsigned int M;
	double Fs, m_lowpass, m_highpass, m_notch, notchWidth;
	bool m_notchOn = false, m_lowpassOn = false, m_highpassOn = false;

public:
	/**
	 * @brief Filter constructor.
	 * @param M The length of the filter and the number of coefficients that
	 * will be returned.
	 * @param Fs The sampling frequency.
	 */
	Filter(unsigned int M, double Fs, double notchWidth = 3) : M(M), Fs(Fs), notchWidth(notchWidth) {}

	/**
	 * @brief Returns a vector with the coefficients.
	 */
	std::vector<T> computeSamples();

	bool isAllpass() { return !(m_lowpassOn || m_highpassOn || m_notchOn); }

	bool lowpassOn() const { return m_lowpassOn; }
	void setLowpassOn(bool on) { m_lowpassOn = on; }
	double lowpass() const { return m_lowpass*Fs/2; }
	void setLowpass(double value) { m_lowpass = value/Fs*2; }

	bool highpassOn() const { return m_highpassOn; }
	void setHighpassOn(bool on) { m_highpassOn = on; }
	double highpass() const { return m_highpass*Fs/2; }
	void setHighpass(double value) { m_highpass = value/Fs*2; }

	bool notchOn() const { return m_notchOn; }
	void setNotchOn(bool on) { m_notchOn = on; }
	double notch() const { return m_notch; }
	void setNotch(double value) { m_notch = value/Fs*2; }
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_FILTER_H
