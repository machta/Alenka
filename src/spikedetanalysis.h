#ifndef SPIKEDETANALYSIS_H
#define SPIKEDETANALYSIS_H

#include <AlenkaSignal/spikedet.h>

namespace AlenkaSignal
{
class OpenCLContext;
template<class T>
class Montage;
}
namespace AlenkaFile
{
class DataFile;
}
class QProgressDialog;

class SpikedetAnalysis
{
public:
	SpikedetAnalysis(AlenkaSignal::OpenCLContext* context) : context(context)
	{}
	~SpikedetAnalysis()
	{
		delete output;
		delete discharges;
	}

	AlenkaSignal::CDetectorOutput* getOutput()
	{
		return output;
	}
	AlenkaSignal::CDischarges* getDischarges()
	{
		return discharges;
	}

	void setSettings(AlenkaSignal::DETECTOR_SETTINGS s)
	{
		settings = s;
	}
	AlenkaSignal::DETECTOR_SETTINGS getSettings()
	{
		return settings;
	}

	void runAnalysis(AlenkaFile::DataFile* file, const std::vector<AlenkaSignal::Montage<float>*>& montage, QProgressDialog* progress);

private:
	AlenkaSignal::OpenCLContext* context;
	AlenkaSignal::DETECTOR_SETTINGS settings;
	AlenkaSignal::CDetectorOutput* output = nullptr;
	AlenkaSignal::CDischarges* discharges = nullptr;
};

#endif // SPIKEDETANALYSIS_H
