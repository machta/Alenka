#include "CSpikeDetector.h"
//#include "CResultsModel.h"

#include <Eigen/Dense>
#include <fasttransforms.h>
#include <interpolation.h>

#include <iostream>

// ------------------------------------------------------------------------------------------------
// CSpikeDetector
// ------------------------------------------------------------------------------------------------

/// A constructor
CSpikeDetector::CSpikeDetector(wxEvtHandler* frame, CInputModel* model, DETECTOR_SETTINGS* settings, CDetectorOutput*& out, CDischarges*& disch)
	: wxThread(), m_out(out), m_discharges(disch)
{
	m_frame = frame;
	m_model = model;
	m_settings = settings;
	m_progress = 0;
}

/// A destructor
CSpikeDetector::~CSpikeDetector()
{
	/* empty */
}

/// This is the entry point of the thread
wxThread::ExitCode CSpikeDetector::Entry()
{    
	wxThreadEvent eventEnd(wxEVT_THREAD, DETECTOR_EVENT);

	try
	{
		RunAnalysis();
	}
	catch (std::exception*& e)
	{
		eventEnd.SetInt(-1); // get exception - all
		eventEnd.SetString(e->what());
		delete e;
		wxQueueEvent((wxEvtHandler*)m_frame, eventEnd.Clone());
		return NULL;

	}
	
	eventEnd.SetInt(-1); // that's all
	wxQueueEvent((wxEvtHandler*)m_frame, eventEnd.Clone());
	return NULL;
}

/// Start the analysis process and store the results.
void CSpikeDetector::RunAnalysis()
{
	int 				  i, j, k, indexSize;
	int 				  start, stop, tmp;
	wxVector<SIGNALTYPE>* segments = NULL;
	wxVector<int>         indexStart, indexStop;

	BANDWIDTH 			  bandwidth(m_settings->m_band_low, m_settings->m_band_high);

	CDetectorOutput*      subOut 		= NULL;
	CDischarges*          subDischarges = NULL;

	int 				  posSize, disSize, tmpFirst, tmpLast;
	double 				  minMP, tmpShift;

	wxVector<int>         removeOut;
	wxVector<int>         removeDish;

	int 				  countSamples = m_model->GetCountSamples();
	int 				  countChannels = m_model->GetCountChannels();
	int 				  fs = m_model->GetFS();

	int    	  			  winsize  = m_settings->m_winsize * fs;

	wxThreadEvent         event(wxEVT_THREAD, DETECTOR_EVENT);
	int 				  progressBy;

	// verify buffering
	tmp = countSamples / fs;
	if (m_settings->m_buffering > tmp)
		m_settings->m_buffering = tmp;

	// Signal buffering
	int N_seg = floor(countSamples/(m_settings->m_buffering * fs));
	if (N_seg < 1) N_seg = 1;
	int T_seg = round((double)countSamples/(double)N_seg/fs);
	// Indexs of segments with two-side overlap
	getIndexStartStop(indexStart, indexStop, countSamples, T_seg, fs, winsize);

	progressBy = round(100/(float)indexStart.size());

	// starting analysis on the segmented data
	indexSize = indexStop.size();
	for (i = 0; i < indexSize; i ++)
	{
		start = indexStart.at(i);
		stop = indexStop.at(i);

		if (TestDestroy())
		{
			std::cout << "cancel" << std::endl;
			break;
		}

		segments = m_model->GetSegment(start, stop);
		if (segments == NULL)
		{
			// error - end of file?
			break;
		}
		spikeDetector(segments, countChannels, fs, bandwidth, subOut, subDischarges);

		delete [] segments;
		segments = NULL;

		// send progress to main frame
		m_progress += progressBy;
		event.SetInt(m_progress);
		wxQueueEvent((wxEvtHandler*)m_frame, event.Clone());

		// removing of two side overlap detections
		posSize = subOut->m_pos.size();
		disSize = subDischarges->m_MP[0].size();

		if (i > 0)
			tmpFirst = 1;
		else tmpFirst = 0;

		if (i < (int)indexStop.size()-1)
			tmpLast = 1;
		else tmpLast = 0;

		if (posSize > 0)
		{
			if (indexStop.size() > 1)
			{
				for (j = 0; j < posSize; j++)
				{
					if (subOut->m_pos.at(j) < tmpFirst*3*m_settings->m_winsize ||
						subOut->m_pos.at(j) > ((stop - start) - tmpLast*3*m_settings->m_winsize*fs)/fs )
						removeOut.push_back(j);
				}
				subOut->Remove(removeOut);

				for (j = 0; j < disSize; j++)
				{
					minMP = INT_MAX;
					for (k = 0; k < countChannels; k++)
						if (subDischarges->m_MP[k].at(j) < minMP)
							minMP = subDischarges->m_MP[k].at(j);

					if (minMP < tmpFirst*3*m_settings->m_winsize ||
						minMP > ((stop-start) - tmpLast*3*m_settings->m_winsize*fs)/fs )
						removeDish.push_back(j);
				}
				subDischarges->Remove(removeDish);
			}
		}

		posSize = subOut->m_pos.size();
		disSize = subDischarges->m_MP[0].size();
		tmpShift = (indexStart.at(i)+1)/(double)fs - 1/(double)fs;

		// connect out
		for (j = 0; j < posSize; j++)
		{
			m_out->Add(
						subOut->m_pos.at(j) + tmpShift,
						subOut->m_dur.at(j),
						subOut->m_chan.at(j),
						subOut->m_con.at(j),
						subOut->m_weight.at(j),
						subOut->m_pdf.at(j)
						);
		}

		// connect discharges
		for (j = 0; j < countChannels; j++)
		{
			for (k = 0; k < (int)subDischarges->m_MP[j].size(); k++)
			{
				subDischarges->m_MP[j].at(k) += tmpShift;
			}
		}

		for (j = 0; j < countChannels; j++)
		{
			m_discharges->m_MV[j].insert(m_discharges->m_MV[j].end(), subDischarges->m_MV[j].begin(), subDischarges->m_MV[j].end());
			m_discharges->m_MA[j].insert(m_discharges->m_MA[j].end(), subDischarges->m_MA[j].begin(), subDischarges->m_MA[j].end());
			m_discharges->m_MP[j].insert(m_discharges->m_MP[j].end(), subDischarges->m_MP[j].begin(), subDischarges->m_MP[j].end());
			m_discharges->m_MD[j].insert(m_discharges->m_MD[j].end(), subDischarges->m_MD[j].begin(), subDischarges->m_MD[j].end());
			m_discharges->m_MW[j].insert(m_discharges->m_MW[j].end(), subDischarges->m_MW[j].begin(), subDischarges->m_MW[j].end());
			m_discharges->m_MPDF[j].insert(m_discharges->m_MPDF[j].end(), subDischarges->m_MPDF[j].begin(), subDischarges->m_MPDF[j].end());
		}
		
		// clear
		if (subOut)
			delete subOut;
		if (subDischarges)
			delete subDischarges;
		removeOut.clear();
		removeDish.clear();
	}
}

/// Calculate the starts and ends of indexes for @see #spikeDetector
void CSpikeDetector::getIndexStartStop(wxVector<int>& indexStart, wxVector<int>& indexStop, const int& cntElemInCh, const double& T_seg,
									   const int& fs, const int& winsize)
{
	int start = 0;
	int i, startSize, end;

	while (start < cntElemInCh)
	{
		indexStart.push_back(start);
		start += T_seg * fs;
	}

	startSize = indexStart.size();
	if (indexStart.size() > 1)
	{
		for (i = 1; i < startSize; i++)
			indexStart.at(i) -= 3*winsize;

		for (i = 0; i < startSize; i++)
		{
			end = indexStart.at(i) + T_seg*fs + 2*(3*winsize);
			indexStop.push_back(end);
		}

		indexStop.front() -= 3*winsize;
		indexStop.back() = cntElemInCh;

		if (indexStop.back() - indexStart.back() < T_seg * fs)
		{
			indexStart.pop_back();
			//indexStart.pop_back(); // No idea what this was supposed to mean...
			//indexStop.back() = cntElemInCh;
			auto tmp = indexStop.back();
			indexStop.pop_back();
			indexStop.back() = tmp;
		}
	}
	else
	{
		indexStop.push_back(cntElemInCh);
	}
}

/// Spike detector
void CSpikeDetector::spikeDetector(wxVector<SIGNALTYPE>*& data, const int& countChannels, const int& inputFS, const BANDWIDTH& bandwidth, 
								   CDetectorOutput*& out, CDischarges*& discharges)
{
	double 				  k1 = m_settings->m_k1;
	double 				  k2 = m_settings->m_k2;
	double 				  discharge_tol = m_settings->m_discharge_tol;
	int 				  decimation = m_settings->m_decimation;
	int                   fs = inputFS;

	int    		  		  countRecords = data[0].size();
	wxVector<int> 		  index;
	int 		  		  stop, step;
	int	  	        	  i, j;
	float 				  k;
	int 				  tmp_start;
	COneChannelDetect**   threads;
	ONECHANNELDETECTRET** ret;
	
	int    	  			  winsize  = m_settings->m_winsize * fs;
	double 	  			  noverlap = m_settings->m_noverlap * fs;

	// OUT
	double 				  t_dur = 0.005;
	wxVector<bool> 		  ovious_M(data[0].size(), false);
	double 				  position;
	bool 				  tmp_sum = false;

	wxVector<double>** 	  m;
	int 				  tmp_round;
	float 				  tmp_start2, tmp_stop;

	// definition of multichannel events vectors
	wxVector<int>* 		  point = new wxVector<int>[2];
	int 				  tmp_old = 0;
	int 				  tmp_act = 0;
	int 				  channel;

	// MV && MA && MW && MPDF && MD && MP
	double 				  tmp_seg;
	double   			  tmp_mv;
	double 				  tmp_max_ma;
	double 				  tmp_max_mw;
	double 				  tmp_max_mpdf;
	double 				  tmp_md;
	double 				  tmp_mp;
	int    				  tmp_row;

	// If sample rate is > "decimation" the signal is decimated => 200Hz default.
	if (fs > decimation)
	{
		CDSP::Resample(data, countChannels, fs, decimation, m_settings->m_original);
		
		fs = decimation;
		winsize  = m_settings->m_winsize * fs;
		noverlap = m_settings->m_noverlap * fs;
		countRecords = data[0].size();
	}

	// Segmentation index
	stop = countRecords - winsize + 1;

	if (noverlap < 1)
		step = round(winsize * (1 - noverlap));
	else
		step = winsize - noverlap;

	for (i = 0; i < stop; i += step)
		index.push_back(i);

	// FILTERING
	// filtering Nx50Hz
	CDSP::Filt50Hz(data, countChannels, fs, m_settings->m_main_hum_freq, bandwidth);

	// filtering 10-60Hz
	CDSP::Filtering(data, countChannels, fs, bandwidth);

	// local maxima detection
	ret = new ONECHANNELDETECTRET*[countChannels];
	threads = new COneChannelDetect*[countChannels];
	for (i = 0; i < countChannels; i++)
	{
		threads[i] = new COneChannelDetect(&data[i], m_settings, fs, &index, i);
		threads[i]->Run();
	}

	for (i = 0; i < countChannels; i++)
	{
		ret[i] = (ONECHANNELDETECTRET*)threads[i]->Wait();
		delete threads[i];
	}

	delete [] threads;
	// processing detection results
	for (i = 0; i < countChannels; i++)
	{
		if (ret[i] == NULL)
			continue;

		//% first and last second is not analyzed (filter time response etc.)
		// first section
		for (j = 0; j < fs; j++)
		{
			ret[i]->m_markersHigh->at(j) = false;
			ret[i]->m_markersLow->at(j) = false;
		}
		// last section
		tmp_start = ret[i]->m_markersHigh->size() - fs - 1;
		for (j = tmp_start; j < (int)ret[i]->m_markersHigh->size(); j++)
		{
			ret[i]->m_markersHigh->at(j) = false;
			ret[i]->m_markersLow->at(j) = false;
		}
	}

	// OUT
	out = new CDetectorOutput();
	for (channel = 0; channel < countChannels; channel++)
	{
		for (j = 0; j < countRecords; j++)
		{
			if (ret[channel] == NULL)
				continue;

			if (ret[channel]->m_markersHigh->at(j) == true)
			{
				ovious_M.at(j) = true;
				position = (j+1)/(double)fs;

				out->Add(position, t_dur, channel + 1, 1, ret[channel]->m_envelopeCdf.at(j), ret[channel]->m_envelopePdf.at(j));
			}
		}
	}

	// ambiguous spike events output
	if (k1 != k2)
	{
		for (channel = 0; channel < countChannels; channel++)
		{
			if (ret[channel] == NULL)
				continue;

			for (j = 0; j < countRecords; j++)
			{
				if (ret[channel]->m_markersLow->at(j) == true)
				{
					if (ret[channel]->m_markersHigh->at(j) == true)
						continue;

					tmp_sum = false;
					for (k = round(j - 0.01*fs); k <= (j - 0.01*fs); k++)
						if(ovious_M.at(k))
							tmp_sum = true;

					if(tmp_sum)
					{
						position = (j+1)/(double)fs;
						out->Add(position, t_dur, channel + 1, 0.5, ret[channel]->m_envelopeCdf.at(j), ret[channel]->m_envelopePdf.at(j));
					}
				}
			}
		}
	}

	// making M stack pointer of events
	m = new wxVector<double>*[countChannels];
	for (i = 0; i < countChannels; i++)
		m[i] = new wxVector<double>(countRecords, 0.0);

	for (i = 0; i < (int)out->m_pos.size(); i++)
	{
		tmp_start2 = out->m_pos.at(i) * fs;
		tmp_stop = out->m_pos.at(i) * fs + discharge_tol * fs;
		for (k = tmp_start2; k <= tmp_stop; k += 1)
		{
			tmp_round = round(k) - 1;
			m[out->m_chan.at(i)-1]->at(tmp_round) = out->m_con.at(i);
		}
	}

	// definition of multichannel events vectors
	delete [] point;
	point = new wxVector<int>[2];
	for (i = 0; i < countRecords; i++)
	{
		tmp_act = 0;
		for (j = 0; j < countChannels; j++)
			if (m[j]->at(i) > 0)
				tmp_act = 1;

		if (tmp_old != tmp_act && tmp_act - tmp_old > 0)
			point[0].push_back(i);
		else if (tmp_old != tmp_act && tmp_act - tmp_old < 0)
			point[1].push_back(i-1);

		tmp_old = tmp_act;
	}

	// MV && MA && MW && MPDF && MD && MP
	discharges = new CDischarges(countChannels);
	for (i = 0; i < (int)point[0].size(); i++)
	{
		for (channel = 0; channel < countChannels; channel++)
		{
			if (ret[channel] == NULL) continue;

			tmp_mv 	     = 0;
			tmp_max_ma   = 0;
			tmp_max_mw   = 0;
			tmp_max_mpdf = 0;
			tmp_mp  	 = NAN;
			tmp_row 	 = 0;

			for (j = point[0].at(i) - 1; j < point[1].at(i); j++)
			{
				// MV
				if (m[channel]->at(j) > tmp_mv)
				{
					tmp_mv = m[channel]->at(j);
					// MP
					if (std::isnan(tmp_mp))
						tmp_mp = ((double)tmp_row + point[0].at(i)+1) / (double)fs;
				}

				// MA
				tmp_seg = ret[channel]->m_envelope.at(j) - (ret[channel]->m_prahInt[0].at(j) / (double)k1);
				tmp_seg = std::fabs(tmp_seg);
				if (tmp_seg > tmp_max_ma)
					tmp_max_ma = tmp_seg;

				// MW
				tmp_seg = ret[channel]->m_envelopeCdf.at(j);
				if (tmp_seg > tmp_max_mw)
					tmp_max_mw = tmp_seg;

				// MPDF
				tmp_seg = ret[channel]->m_envelopePdf.at(j) * m[channel]->at(j);
				if (tmp_seg > tmp_max_mpdf)
					tmp_max_mpdf = tmp_seg;

				tmp_row++;
			}

			discharges->m_MV[channel].push_back(tmp_mv);
			discharges->m_MA[channel].push_back(tmp_max_ma);
			discharges->m_MPDF[channel].push_back(tmp_max_mpdf);
			discharges->m_MW[channel].push_back(tmp_max_mw);
			discharges->m_MP[channel].push_back(tmp_mp);

			// MD
			tmp_md = (point[1].at(i) - point[0].at(i)) / (double)fs;
			discharges->m_MD[channel].push_back(tmp_md);
		}
	}

	for (i = 0; i < countChannels; i++)
	{
		if (ret[i] == NULL) continue;

		delete ret[i];
		delete m[i];
	}

	delete [] point;
	delete [] m;
	delete [] ret;
}

// ------------------------------------------------------------------------------------------------
// COneChannelDetect 
// ------------------------------------------------------------------------------------------------

/// A constructor
COneChannelDetect::COneChannelDetect(const wxVector<SIGNALTYPE>* data, const DETECTOR_SETTINGS* settings, const int& fs, const wxVector<int>* index,
									 const int& channel)
	: wxThread(wxTHREAD_JOINABLE), m_data(data), m_settings(settings), m_fs(fs), m_index(index), m_channel(channel)
{
	/* empty */
}

/// A destructor
COneChannelDetect::~COneChannelDetect()
{
	/* empty */
}

/// This is the entry point of the thread.
wxThread::ExitCode COneChannelDetect::Entry()
{
	
	wxVector<SIGNALTYPE>  envelope(m_data->begin(), m_data->end());
	int 				  start, stop, tmp, i, j;
	int 				  indexSize = m_index->size();
	int     			  envelopeSize = envelope.size();
	double 				  std, l, m;

	wxVector<double>      logs;
	alglib::real_1d_array r1a_logs;

	wxVector<SIGNALTYPE>  phatMedian, phatStd;

	ONECHANNELDETECTRET*  ret = NULL;

	// Hilbert's envelope (intense envelope)
	CDSP::AbsHilbert(envelope);

	for (i = 0; i < indexSize; i++)
	{
		start = m_index->at(i);
		stop = start + m_settings->m_winsize*m_fs - 1;

		for (j = start; j <= stop; j++)
		{
			if (envelope[j] > 0)
			{
				l = log(envelope[j]);
				logs.push_back(l);
			}
		}

		m = mean(logs);
		std = sqrt(variance(logs, m));

		phatMedian.push_back(m);
		phatStd.push_back(std);
		logs.clear();
	}

	double r = (double)envelope.size() / (double)indexSize;
	double n_average = m_settings->m_winsize;

	wxVector<double> b, a;
	tmp = round(n_average*m_fs/r);

	if (tmp > 1)
	{
		a.push_back(1);
		for (i = 0; i < tmp; i++)
		{
			l = 1.0/tmp;
			b.push_back(l);
		}

		CDSP::FiltFilt(b, a, &phatMedian);
		CDSP::FiltFilt(b, a, &phatStd);
	}

	// interpolation of thresholds value to threshold curve (like backround)
	wxVector<double> phat_int[2];
	wxVector<double> x, y;
	alglib::real_1d_array xreal, yrealMedian, yrealStd, x2real, retMedian, retStd;

	if (phatMedian.size() > 1)
	{
		for (i = 0; i < indexSize; i++)
			x.push_back( m_index->at(i) + round( (m_settings->m_winsize * m_fs)/2 ) );

		start = *m_index->begin();
		stop = m_index->back();
		y.reserve(m_index->back());
		for (i = start; i <= stop; i++)
			y.push_back(i + round( (m_settings->m_winsize * m_fs)/2 ));

		try
		{
			// interpolation Median and Std
			wxVector<double> phatMedVecDoub(phatMedian.begin(), phatMedian.end());
			wxVector<double> phatStdVecDoub(phatStd.begin(), phatStd.end());

			xreal.setcontent(x.size(), &x[0]);
			yrealMedian.setcontent(phatMedVecDoub.size(), &phatMedVecDoub[0]);
			yrealStd.setcontent(phatStdVecDoub.size(), &phatStdVecDoub[0]);
			x2real.setcontent(y.size(), &y[0]);

			alglib::spline1dconvcubic(xreal, yrealMedian, x2real, retMedian);
			alglib::spline1dconvcubic(xreal, yrealStd, x2real, retStd);
		}
		catch(alglib::ap_error e)
		{
			std::cerr << "Aglib warning: " << e.msg.c_str() << std::endl;
			//return NULL;
			// This NULL returned after the exception caused the error for the 130 channel file
			// that has one channel with all zeroes. So, rather than to return an empty result,
			// fill it with zeroes.

			assert(yrealMedian.length() == yrealStd.length());
			assert(retMedian.length() == 0 && retStd.length() == 0);

			wxVector<double> allZeroes(yrealMedian.length());
			retMedian.setcontent(allZeroes.size(), allZeroes.data());
			retStd.setcontent(allZeroes.size(), allZeroes.data());
		}

		for (i = 0; i < retMedian.length(); i++)
		{
			phat_int[0].push_back(retMedian[i]);
			phat_int[1].push_back(retStd[i]);
		}

		// DOPLNENI
		double temp_elem0 = phat_int[0].front();
		double temp_elem1 = phat_int[1].front();

//		for (i = 0; i < floor(m_settings->m_winsize * m_fs / 2); i++)
//		{
//			phat_int[0].insert(phat_int[0].begin(), temp_elem0);
//			phat_int[1].insert(phat_int[1].begin(), temp_elem1);
//		}
// My optimization: insert constants at one to prevent repeated copying in the vector. Makes is about 2.5x faster.
		int n = floor(m_settings->m_winsize * m_fs / 2);
		phat_int[0].insert(phat_int[0].begin(), n, temp_elem0);
		phat_int[1].insert(phat_int[1].begin(), n, temp_elem1);

		temp_elem0 = phat_int[0].back();
		temp_elem1 = phat_int[1].back();
		for (i = phat_int[0].size(); i < envelopeSize; i++)
		{
			phat_int[0].push_back(temp_elem0);
			phat_int[1].push_back(temp_elem1);
		}
	}
	else
	{
		for (i = 0; i < envelopeSize; i++)
		{
			phat_int[0].push_back( phatMedian.at(1) * 1 );
			phat_int[1].push_back( phatStd.at(1) * 1 );
		}
	}

	// LOGNORMAL distr.
	double tmp_diff, tmp_exp, tmp_square, lognormal_mode, lognormal_median, tmp_prah_int, lognormal_mean, tmp_sum;
	wxVector<double> prah_int[2];

	double tmp_sqrt_one, tmp_to_erf, tmp_log, tmp_erf, tmp_pdf, tmp_x, tmp_x2;
	double tmp_sqrt = sqrt(2*M_PI);
	wxVector<double> envelope_cdf, envelope_pdf;
	int phatIntSize = phat_int[0].size();

	for (i = 0; i < phatIntSize; i++)
	{
		tmp_square = phat_int[1].at(i) * phat_int[1].at(i);
		tmp_diff   = phat_int[0].at(i) - tmp_square;
		tmp_sum    = phat_int[0].at(i) + tmp_square/2;

		lognormal_mode   = exp(tmp_diff);
		lognormal_median = exp(phat_int[0].at(i));
		lognormal_mean   = exp(tmp_sum);

		tmp_prah_int = (m_settings->m_k1 * (lognormal_mode + lognormal_median)) - (m_settings->m_k3 * (lognormal_mean-lognormal_mode));
		prah_int[0].push_back(tmp_prah_int);

		if (m_settings->m_k2 != m_settings->m_k1)
		{
			tmp_prah_int = (m_settings->m_k2 * (lognormal_mode + lognormal_median)) - (m_settings->m_k3 * (lognormal_mean-lognormal_mode));
			prah_int[1].push_back(tmp_prah_int);
		}

		// CDF of lognormal distribution, PDF of lognormal distribution
		// CDF
		tmp_sqrt_one = sqrt( 2.0f * phat_int[1].at(i) * phat_int[1].at(i));
		tmp_log = log(envelope[i]);
		tmp_to_erf = (tmp_log - phat_int[0].at(i)) / tmp_sqrt_one;
		tmp_erf = erf(tmp_to_erf);
		envelope_cdf.push_back(0.5 + 0.5 * tmp_erf);

		// PDF
		tmp_x = (tmp_log - phat_int[0].at(i)) / phat_int[1].at(i);
		tmp_x *= tmp_x;
		tmp_exp = exp( -0.5 * tmp_x );
		tmp_x2 = (envelope[i] * phat_int[1].at(i) * tmp_sqrt);
		tmp_pdf = tmp_exp / tmp_x2;
		envelope_pdf.push_back(tmp_pdf);
	}

	wxVector<bool>* markers_high = NULL,* markers_low = NULL;
	try {
		markers_high = localMaximaDetection(envelope, prah_int[0], m_settings->m_polyspike_union_time);
		detectionUnion(markers_high, envelope, m_settings->m_polyspike_union_time * m_fs);

		if (m_settings->m_k2 != m_settings->m_k1 && prah_int[1].size() != 0)
		{
			markers_low = localMaximaDetection(envelope, prah_int[1], m_settings->m_polyspike_union_time);
			detectionUnion(markers_low, envelope, m_settings->m_polyspike_union_time * m_fs);
		} else markers_low = markers_high;
	}
	catch (CException* e)
	{
		return NULL;
	}

	ret = new ONECHANNELDETECTRET(markers_high, markers_low, prah_int, envelope_cdf, envelope_pdf, envelope);
	return ret;
}

/// Calculating a mean from data in vector
double COneChannelDetect::mean(wxVector<double>& data) 
{
	float sum = 0;
	wxVector<double>::iterator b = data.begin();
	wxVector<double>::iterator e = data.end();

	while (b != e)
	{
		sum = sum + *b;
		++b;
	}

	return sum / data.size();
}

/// Calculating a variance from data in vector
double COneChannelDetect::variance(wxVector<double>& data, const double & mean)
{   
	wxVector<double> v(data.begin(), data.end());

	wxVector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
				   std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);

	return sq_sum / (v.size()-1);
}

/// Detection of local maxima in envelope
wxVector<bool>* COneChannelDetect::localMaximaDetection(wxVector<SIGNALTYPE>& envelope, const wxVector<double>& prah_int, const double& polyspike_union_time)
{
	unsigned int         size = envelope.size();
	wxVector<bool>* 	 marker1 = new wxVector<bool>(size, 0); // This leak was fixed in ~oneChannelDetectRet().
	wxVector<int>   	 point[2];
	wxVector<SIGNALTYPE> seg, seg_s;
	wxVector<int>        tmp_diff_vector;
	int        			 pointer_max;
	SIGNALTYPE 			 tmp_max;
	int 				 tmp_pointer;
	unsigned int 		 i, j, tmp;

	wxVector<int> 		 pointer;
	bool 				 state_previous = false;
	int 				 tmp_ceil, tmp_stop, start;
	float 				 tmp_sum_elems;

	wxVector<int> 		 lokal_max, lokal_max_poz;
	wxVector<SIGNALTYPE> lokal_max_val;
	float 				 tmp_diff;
	int 				 tmp_sign, tmp_sign_next;
	wxVector<float> 	 tmp_diff_vector2;

	for (i = 0; i < size; i++)
		if (envelope[i] > prah_int[i])
			marker1->at(i) = 1;

	// start + end crossing
	findStartEndCrossing(point, marker1);
	if (point[0].size() != point[1].size())
		throw new CException(wxT("local_maxima_detection: point sizes are different"), wxT("COneChannelDetect::localMaximaDetection"));

	marker1->assign(size, 0);

	for (i = 0; i < point[0].size(); i++)
	{
		seg_s.clear();
		seg.clear();
		tmp_diff_vector.clear();

		// detection of local maxima in section which crossed threshold curve
		if (point[1].at(i) - point[0].at(i) > 2)
		{
			seg.assign(&envelope[0] + point[0].at(i), &envelope[0] + point[1].at(i)+1);
			for (j = 0; j < seg.size()-1; j++)
				seg_s.push_back(seg.at(j+1) - seg.at(j));

			for (j = 0; j < seg_s.size(); j++)
			{
				if (seg_s.at(j) > 0) seg_s.at(j) = 1;
				else if(seg_s.at(j) < 0) seg_s.at(j) = -1;
				else seg_s.at(j) = 0;
			}

			// positions of local maxima in the section
			seg_s.insert(seg_s.begin(), 0);
			for (j = 0; j < seg_s.size()-1; j++)
				tmp_diff_vector.push_back(seg_s.at(j+1) - seg_s.at(j));

			seg_s.clear();
			for (j = 0; j < tmp_diff_vector.size(); j++)
				if (tmp_diff_vector.at(j) < 0)
					seg_s.push_back(j);

			for (j = 0; j < seg_s.size(); j++)
			{
				tmp_pointer = point[0].at(i) + seg_s.at(j);
				if (tmp_pointer < (int)size)
					marker1->at(tmp_pointer) = 1;
			}
		}
		else if (point[1].at(i) - point[0].at(i) <= 2)
		{
			pointer_max = 1;
			tmp_max = 0;
			seg.assign(&envelope[0] + point[0].at(i), &envelope[0] + point[1].at(i)+1);
			for (j = 0; j < seg.size(); j++)
			{
				if (seg.at(j) > tmp_max)
				{
					pointer_max = j;
					tmp_max = seg.at(j);
				}
			}

			tmp = point[0].at(i) + pointer_max;
			if (tmp < size)
				marker1->at(tmp) = 1;
		}
	}

	// union of section, where local maxima are close together <(1/f_low + 0.02 sec.)~ 120 ms
	for (i = 0; i < marker1->size(); i++)
		if (marker1->at(i))
			pointer.push_back(i);

	state_previous = false;
	for (i = 0; i < pointer.size(); i++)
	{
		tmp_sum_elems = 0;
		seg.clear();

		tmp_ceil = ceil(pointer.at(i) + polyspike_union_time * m_fs);
		if (tmp_ceil >= (int)size)
			tmp_stop = size;
		else
			tmp_stop = tmp_ceil + 1;

		for(j = pointer.at(i)+1; j < (unsigned)tmp_stop; j++)
			seg.push_back(marker1->at(j));

		for (wxVector<SIGNALTYPE>::iterator j = seg.begin() ; j != seg.end(); ++j)
			tmp_sum_elems += *j;

		if (state_previous)
		{
			if (tmp_sum_elems > 0)
				state_previous = true;
			else
			{
				state_previous = false;
				for (j = start; j <= (unsigned)pointer.at(i) && j < size; j++)
					marker1->at(j) = true;
			}
		}
		else
		{
			if (tmp_sum_elems > 0)
			{
				state_previous = true;
				start = pointer.at(i);
			}
		}
	}

	// finding of the highes maxima of the section with local maxima
	findStartEndCrossing(point, marker1);

	if (point[0].size() != point[1].size())
		throw new CException(wxT("local_maxima_detection: point sizes are different 2"), wxT("COneChannelDetect::localMaximaDetection"));

	// local maxima with gradient in souroundings
	for (i = 0; i < point[0].size(); i++)
	{
		lokal_max.clear();
		lokal_max_val.clear();
		lokal_max_poz.clear();

		tmp_diff = point[1].at(i) - point[0].at(i);
		if (tmp_diff > 1)
		{
			for (j = 0; j < pointer.size(); j++)
				if (pointer.at(j) >= point[0][i] && pointer.at(j) <= point[1][i])
					lokal_max.push_back(pointer.at(j));

			for (j = point[0].at(i); j <= (unsigned)point[1].at(i) && j < size; j++)
				marker1->at(j) = false;

			// envelope magnitude in local maxima
			for (j = 0; j < lokal_max.size(); j++)
				lokal_max_val.push_back(envelope[lokal_max.at(j)]);

			// lokal_max_poz=(diff(sign(diff([0;lokal_max_val;0]))<0)>0);
			// diff([0;lokal_max_val;0])
			tmp_diff_vector2.clear();
			tmp_diff = 0;
			for (j = 0; j < lokal_max_val.size(); j++)
			{
				tmp_diff_vector2.push_back( lokal_max_val.at(j) - tmp_diff );
				tmp_diff = lokal_max_val.at(j);
			}
			tmp_diff_vector2.push_back( 0 - lokal_max_val.back() );

			// sign(diff([0;lokal_max_val;0]))<0
			lokal_max_poz.clear();
			for (j = 0; j < tmp_diff_vector2.size()-1; j++)
			{
				if (tmp_diff_vector2.at(j) > 0) tmp_sign = 1;
				else tmp_sign = 0;

				if (tmp_diff_vector2.at(j+1) > 0) tmp_sign_next = 1;
				else tmp_sign_next = 0;

				tmp_diff = tmp_sign - tmp_sign_next;
				lokal_max_poz.push_back(tmp_diff);
			}

			for (j = 0; j < lokal_max.size(); j++)
			{
				if (lokal_max_poz.at(j) == 1 && lokal_max.at(j) < (int)size)
				{
					marker1->at( lokal_max.at(j) ) = true;
				}
			}
		}
	}

	return marker1;
}

/// Detecting of union and their merging.
void COneChannelDetect::detectionUnion(wxVector<bool>* marker1, wxVector<SIGNALTYPE>& envelope, const double& union_samples)
{
	int 				  i, j, start, stop, sum = round(union_samples);
	float 			 	  max; // maximum value in segment of envelope
	int 			 	  max_pos; // position of maximum
	wxVector<double> 	  vec_MASK(sum, 1.0);
	wxVector<double> 	  vec_marker2(marker1->begin(), marker1->end());
	wxVector<int>    	  point[2];
	alglib::real_1d_array r1a_marker2, r1a_MASK, r1a_ret;

	// dilatation
	r1a_marker2.setcontent(vec_marker2.size(), &vec_marker2[0]);
	r1a_MASK.setcontent(vec_MASK.size(), &vec_MASK[0]);
	vec_marker2.clear();
	marker1->assign(marker1->size(), false);
	alglib::convr1d(r1a_marker2, r1a_marker2.length(), r1a_MASK, r1a_MASK.length(), r1a_ret);

	start = (vec_MASK.size() / 2);
	stop  = r1a_ret.length() - (vec_MASK.size() - start);

	for (i = start; i <= stop; i++)
		if (r1a_ret[i] > 0)
			marker1->at(i-start) = true;

	// erosion
	vec_marker2.assign(marker1->begin(), marker1->end());
	marker1->assign(marker1->size(), false);
	r1a_marker2.setcontent(vec_marker2.size(), &vec_marker2[0]);
	alglib::convr1d(r1a_marker2, r1a_marker2.length(), r1a_MASK, r1a_MASK.length(), r1a_ret);

	stop  = r1a_ret.length() - (vec_MASK.size() - start);
	for (i = start; i <= stop; i++)
		if (r1a_ret[i] == sum)
			marker1->at(i-start) = true;

	// start + end crossing
	findStartEndCrossing(point, marker1);

	marker1->assign(marker1->size(), false);
	for (i = 0; i < (int)point[0].size(); i++)
	{
		max = -1;
		for (j = point[0].at(i); j <= point[1].at(i); j++)
		{
			if (envelope[j] > max)
			{
				max = envelope[j];
				max_pos = j;
			}
		}

		marker1->at(max_pos) = true;
	}
}

// Finding of the highes maxima of the section with local maxima
void COneChannelDetect::findStartEndCrossing(wxVector<int> point[2], const wxVector<bool>* marker1)
{
	bool one = false;
	bool tmp_marker;
	int  size = marker1->size();
	int i;

	point[0].clear();
	point[1].clear();

	for (i = 0; i < size; i++)
	{
		tmp_marker = marker1->at(i);
		if (!one) // start crossing
		{
			if (tmp_marker == 1)
			{
				one = true;
				point[0].push_back(i);

				if (i == size-1)
					point[1].push_back(i);
			}
		} else { // end crossing
			if (tmp_marker == 0 || i == size-1)
			{
				one = false;
				point[1].push_back(i-1);
			}
		}
	}
}
