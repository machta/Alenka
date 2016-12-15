#ifndef SPIKEDETANALYSIS_H
#define SPIKEDETANALYSIS_H

#include "spikedet.h"

class DataFile;
template<class T>
class Montage;
class OpenCLContext;

class SpikedetAnalysis
{
public:
	SpikedetAnalysis(OpenCLContext* context) : context(context)
	{}
	~SpikedetAnalysis()
	{
		delete output;
		delete discharges;
	}

	CDetectorOutput* getOutput()
	{
		return output;
	}
	CDischarges* getDischarges()
	{
		return discharges;
	}

	void setSettings(DETECTOR_SETTINGS s)
	{
		settings = s;
	}
	DETECTOR_SETTINGS getSettings()
	{
		return settings;
	}

	void runAnalysis(DataFile* file, const std::vector<Montage<float>*>& montage);

private:
	OpenCLContext* context;
	DETECTOR_SETTINGS settings;
	CDetectorOutput* output = nullptr;
	CDischarges* discharges = nullptr;
};

#endif // SPIKEDETANALYSIS_H
