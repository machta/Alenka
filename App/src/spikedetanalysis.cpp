#include "spikedetanalysis.h"

#include "DataFile/datafile.h"
#include "montage.h"

using namespace std;

namespace
{

template<class T>
class Loader : public SpikedetDataLoader<T>
{
	DataFile* file;
	vector<T> tmpData;

public:
	Loader(DataFile* file) : file(file)
	{
	}
	virtual ~Loader() override
	{}

	virtual void readSignal(T* data, int64_t firstSample, int64_t lastSample) override
	{
		int64_t len = (lastSample - firstSample + 1)*channelCount();
		tmpData.resize(len);

		file->readData(&tmpData, firstSample, lastSample);

		for (int64_t i = 0; i < len; i++) // TODO: remove this needless copying after the new File lib is integrated
			data[i] = tmpData[i];
	}
	virtual int64_t sampleCount() override
	{
		return file->getSamplesRecorded();
	}
	virtual int channelCount() override
	{
		return file->getChannelCount();
	}
};

} // namespace

void SpikedetAnalysis::runAnalysis(DataFile* file, Montage<float>* /*montage*/)
{
	assert(file != nullptr);
	//assert(montage != nullptr);

	Spikedet<float> spikedet(file->getSamplingFrequency(), file->getChannelCount(), settings, context);
	Loader<float> loader(file);

	delete output;
	output = new CDetectorOutput;

	delete discharges;
	discharges = new CDischarges(file->getChannelCount());

	spikedet.runAnalysis(&loader, output, discharges);

	int outCount = output->m_pos.size();
}
