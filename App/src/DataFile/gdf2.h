#include "datafile.h"

#include <cstdio>

#ifndef GDF2_H
#define GDF2_H

class GDF2 : public DataFile
{
public:
	GDF2(const char* filePath);
	virtual ~GDF2();

	virtual double getSamplingFrequency()
	{
		return samplingFrequency;
	}
	virtual unsigned int getChannelCount()
	{
		return fh.numberOfChannels;
	}
	virtual uint64_t getSamplesRecorded()
	{
		return samplesRecorded;
	}
	virtual void readData(float* data, int64_t firstSample, int64_t lastSample)
	{
		readDataLocal(data, firstSample, lastSample);
	}
	virtual void readData(double* data, int64_t firstSample, int64_t lastSample)
	{
		readDataLocal(data, firstSample, lastSample);
	}

private:
	FILE* file;
	double samplingFrequency;
	uint64_t samplesRecorded;
	int startOfData;
	bool isLittleEndian;
	double* scale;
	int dataTypeSize;
	double (* convertSampleToDouble)(void* sample);
	float (* convertSampleToFloat)(void* sample);
	int version;
    bool uncalibrated;

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
	void readDataLocal(T* data, int64_t firstSample, int64_t lastSample);
	template<typename T>
	void readFile(T* val, int elements = 1);
	void seekFile(size_t position, bool fromStart = false);
};

#endif // GDF2_H
