#ifndef ALENKAFILE_EDF_H
#define ALENKAFILE_EDF_H

#include "datafile.h"

class edf_hdr_struct;

namespace AlenkaFile
{

/**
 * @brief A class implementing the EDF+ and BDF+ types.
 *
 * There is a limit on the channel count (512) due to the limitations
 * of the EDFlib library.
 */
class EDF : public DataFile
{
	double samplingFrequency;
	int numberOfChannels;
	uint64_t samplesRecorded;
	edf_hdr_struct* edfhdr;
	int readChunk;
	double* readChunkBuffer;

public:
	/**
	 * @brief
	 * @param filePath The file path of the primary data file.
	 */
	EDF(const std::string& filePath);
	virtual ~EDF();

	virtual double getSamplingFrequency() const override
	{
		return samplingFrequency;
	}
	virtual unsigned int getChannelCount() const override
	{
		return numberOfChannels;
	}
	virtual uint64_t getSamplesRecorded() const override
	{
		return samplesRecorded;
	}
	virtual double getStartDate() const override;
	virtual void save() override;
	virtual bool load() override;
	virtual void readChannels(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readChannelsFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readChannels(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readChannelsFloatDouble(dataChannels, firstSample, lastSample);
	}

	virtual double getPhysicalMaximum(unsigned int channel) override;
	virtual double getPhysicalMinimum(unsigned int channel) override;
	virtual double getDigitalMaximum(unsigned int channel) override;
	virtual double getDigitalMinimum(unsigned int channel) override;
	virtual std::string getLabel(unsigned int channel);

	static void saveAs(const std::string& filePath, DataFile* sourceFile);

private:
	template<typename T>
	void readChannelsFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);
	void openFile();
	void fillDefaultMontage();
	void loadEvents();
	void addUsedEventTypes();
	static void saveAsWithType(const std::string& filePath, DataFile* sourceFile, const edf_hdr_struct* edfhdr);
};

} // namespace AlenkaFile

#endif // ALENKAFILE_EDF_H
