#include "../include/AlenkaSignal/filter.h"

#include <complex>

using namespace std;

namespace AlenkaSignal
{

template<class T>
vector<T> Filter<T>::computeSamples()
{
	vector<T> samples((M + 1)/2);

	// Initialize samples with the values of Hr.
	for (unsigned int i = 0; i < samples.size(); ++i)
	{
		double f = 2.*i/M;
		double val = 1;

		if (m_lowpassOn && f >= m_lowpass - 2/Fs*2)
		{
			val = 0;
		}
		else if (m_highpassOn && f <= m_highpass + 1/Fs*2)
		{
			val = 0;
		}
		else if (m_notchOn)
		{
			double tmp = round(f/m_notch);

			if (0 < tmp) // This is to skip the notch at 0 Hz.
			{
				tmp = fabs(f - tmp*m_notch);
				if (tmp <= notchWidth/M*Fs/M)
				{
					val = 0;
				}
			}
		}

		samples[i] = val;
	}

	return samples;
}

template class Filter<float>;
template class Filter<double>;

} // namespace AlenkaSignal
