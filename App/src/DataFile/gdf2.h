#include "datafile.h"

#include "../error.h"

#include <QCache>

#include <cstdio>
#include <mutex>
#include <functional>

#ifndef GDF2_H
#define GDF2_H

class GDF2 : public DataFile
{
public:
	GDF2(const std::string& filePath);
	virtual ~GDF2();

	virtual double getSamplingFrequency() const override
	{
		return samplingFrequency;
	}
	virtual unsigned int getChannelCount() const override
	{
		return fh.numberOfChannels;
	}
	virtual uint64_t getSamplesRecorded() const override
	{
		return samplesRecorded;
	}
	virtual void save() override;
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) override
	{
		readDataLocal(data, firstSample, lastSample);
	}
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) override
	{
		readDataLocal(data, firstSample, lastSample);
	}

protected:
	virtual bool load() override;

private:
	std::mutex fileMutex;
	FILE* file;
	double samplingFrequency;
	uint64_t samplesRecorded;
	int startOfData;
	int startOfEventTable;
	bool isLittleEndian;
	double* scale;
	int dataTypeSize;
	std::function<double (void*)> convertSampleToDouble;
	int version;
	bool uncalibrated;
	QCache<unsigned int, std::vector<char>>* cache = nullptr;

	struct
	{
		char versionID[8 + 1];
		char patientID[66 + 1];
		// 10B reserved
		char drugs;
		uint8_t weight;
		uint8_t height;
		char patientDetails;
		char recordingID[64 + 1];
		uint32_t recordingLocation[4];
		uint32_t startDate[2];
		uint32_t birthday[2];
		uint16_t headerLength;
		char ICD[6];
		uint64_t equipmentProviderID;
		// 6B reserved
		uint16_t headsize[3];
		float positionRE[3];
		float positionGE[3];
		int64_t numberOfDataRecords;
		char durationOfDataRecord[8];
		uint16_t numberOfChannels;
		// 2B reserved
	} fh;

	struct
	{
		char (* label)[16 + 1];
		char (* typeOfSensor)[80 + 1];
		// physicalDimension obsolete
		uint16_t* physicalDimensionCode;
		double* physicalMinimum;
		double* physicalMaximum;
		double* digitalMinimum;
		double* digitalMaximum;
		// prefiltering obsolete
		float* timeOffset;
		float* lowpass;
		float* highpass;
		float* notch;
		uint32_t* samplesPerRecord;
		uint32_t* typeOfData;
		float (* sensorPosition)[3];
		char (* sensorInfo)[20];
	} vh;

	template<typename T>
	void readDataLocal(std::vector<T>* data, int64_t firstSample, int64_t lastSample);
	template<typename T>
	void readFile(T* val, int elements = 1)
	{
		freadChecked(val, sizeof(T), elements, file);

		if (isLittleEndian == false)
		{
			for (int i = 0; i < elements; ++i)
			{
				changeEndianness(val + i);
			}
		}
	}
	void seekFile(size_t offset, bool fromStart = false)
	{
		int res = std::fseek(file, static_cast<long>(offset), fromStart ? SEEK_SET : SEEK_CUR);
		checkErrorCode(res, 0, "seekFile(" << offset << ", " << fromStart << ") failed.");
	}
	template<typename T>
	void writeFile(const T* val, int elements = 1)
	{
		if (isLittleEndian == false)
		{
			for (int i = 0; i < elements; ++i)
			{
				T tmp = val[i];
				changeEndianness(&tmp);
				fwriteChecked(&tmp, sizeof(T), 1, file);
			}
		}
		else
		{
			fwriteChecked(const_cast<T*>(val), sizeof(T), elements, file);
		}
	}
};

#endif // GDF2_H
