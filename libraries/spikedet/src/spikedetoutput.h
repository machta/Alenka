#ifndef SPIKEDETOUTPUT_H
#define	SPIKEDETOUTPUT_H

#include "wx/wx.h"

#include <vector>

/**
 * Output class containing output data from the detector.
 */
class CDetectorOutput
{
	// methods
public:
	/**
	 * A constructor.
	 */
	CDetectorOutput() {}
	
	/**
	 * A virtual desctructor.
	 */
	virtual ~CDetectorOutput() {}

	/**
	 * Add data to the vectors.
	 * @param pos spike position (second).
	 * @param dur spike duration (second) - fix value 5 ms.
	 * @param chan channel.
	 * @param con spike condition (1-obvious 0.5-ambiguous).
	 * @param weight statistical significance "CDF".
	 * @param pdf sstatistical significance "PDF".
	 */
	void Add(const double& pos, const double& dur, const int& chan, const double& con, const double& weight, const double& pdf)
	{
		m_pos.push_back(pos);
		m_dur.push_back(dur);
		m_chan.push_back(chan);
		m_con.push_back(con);
		m_weight.push_back(weight);
		m_pdf.push_back(pdf);
	}

	/**
	 * Erase records at positions.
	 * @param pos position of records to erase.
	 */
	void Remove(const wxVector<int>& pos)
	{
		unsigned i, counter = 0;

		for (i = 0; i < pos.size(); i++)
		{
			int index = pos.at(i) - counter;
			if (static_cast<int>(m_pos.size()) < index || index < 0)
				continue;

			m_pos.erase(m_pos.begin() + index);
			m_dur.erase(m_dur.begin() + index);
			m_chan.erase(m_chan.begin() + index);
			m_con.erase(m_con.begin() + index);
			m_weight.erase(m_weight.begin() + index);
			m_pdf.erase(m_pdf.begin() + index);

			counter++;
		}
	}

private:

	// variables
public:
	/// spike position (second)
	wxVector<double>  m_pos;
	/// channel
	wxVector<int>     m_chan;
	/// spike duration (second) - fix value 5 ms
	wxVector<double>  m_dur;
	/// spike condition (1-obvious 0.5-ambiguous)
	wxVector<double>  m_con;
	/// statistical significance "CDF"
	wxVector<double>  m_weight;
	/// statistical significance "PDF"
	wxVector<double>  m_pdf;
private: 
	/* none */
};

/**
 * Discharges
 */
class CDischarges
{
	// methods
public:
	/**
	 * A constructor.
	 * @param countChannels count channels.
	 */
	CDischarges(const int& countChannels)
	{
		m_countChannels = countChannels;

		m_MV   = new std::vector<double>[countChannels];
		m_MA   = new std::vector<double>[countChannels];
		m_MP   = new std::vector<double>[countChannels];
		m_MD   = new std::vector<double>[countChannels];
		m_MW   = new std::vector<double>[countChannels];
		m_MPDF = new std::vector<double>[countChannels];
	}

	/**
	 * A virtual desctructor.
	 */
	virtual ~CDischarges()
	{
		delete [] m_MV;
		delete [] m_MA;
		delete [] m_MP;
		delete [] m_MD;
		delete [] m_MW;
		delete [] m_MPDF;
	}

	/**
	 * Erase records at positions.
	 * @param pos positions of record to erase.
	 */
	void Remove(const wxVector<int>& pos)
	{
		unsigned i, channel, counter = 0;

		for (i = 0; i < pos.size(); i++)
		{

			for (channel = 0; channel < m_countChannels; channel++)
			{
				int index = pos.at(i) - counter;
				if (static_cast<int>(m_MV[channel].size()) < index || index < 0)
					continue;

				m_MV[channel].erase(m_MV[channel].begin() + index);
				m_MA[channel].erase(m_MA[channel].begin() + index);
				m_MP[channel].erase(m_MP[channel].begin() + index);
				m_MW[channel].erase(m_MW[channel].begin() + index);
				m_MPDF[channel].erase(m_MPDF[channel].begin() + index);
				m_MD[channel].erase(m_MD[channel].begin() + index);
			}
			counter++;
		}

	}

	/**
	  *	Return count channels.
	  * @return count channels.s
	  */
	inline unsigned GetCountChannels() const
	{
		return m_countChannels;
	}
private:

	// variables
public:
	/// spike type 1-obvious, 0.5- ambiguous
	std::vector<double>* m_MV;
	/// max. amplitude of envelope above backround
	std::vector<double>* m_MA;
	/// event start position
	std::vector<double>* m_MP;
	/// duration of event
	std::vector<double>* m_MD;
	/// statistical significance "CDF"
	std::vector<double>* m_MW;
	/// probability of occurence
	std::vector<double>* m_MPDF;
private:
	/// count channels
	unsigned 			 m_countChannels;
};

#endif // SPIKEDETOUTPUT_H
