#include "gdf2.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <cassert>

using namespace std;

GDF2::GDF2(const char* filePath)
{
	isLittleEndian = testLittleEndian();

	file = fopen(filePath, "r+b");
	assert(file != NULL); // test whether the file exists, if it was opened properly, if it is gdf2

	// Load fixed header.
	seekFile(0, true);

	readFile(fixedHeader.versionID, 8);
	fixedHeader.versionID[8] = 0;

	readFile(fixedHeader.patientID, 66);
	fixedHeader.patientID[66] = 0;

	seekFile(10);

	readFile(&fixedHeader.drugs);

	readFile(&fixedHeader.weight);

	readFile(&fixedHeader.height);

	readFile(&fixedHeader.patientDetails);

	readFile(fixedHeader.recordingID, 64);
	fixedHeader.recordingID[64] = 0;

	readFile(fixedHeader.recordingLocation, 4);

	readFile(fixedHeader.startDate, 2);

	readFile(fixedHeader.birthday, 2);

	readFile(&fixedHeader.headerLength);

	readFile(fixedHeader.ICD, 6);

	readFile(&fixedHeader.equipmentProviderID);

	seekFile(6);

	readFile(fixedHeader.headsize, 3);

	readFile(fixedHeader.positionRE, 3);

	readFile(fixedHeader.positionGE, 3);

	readFile(&fixedHeader.numberOfDataRecords);

	readFile(fixedHeader.durationOfDataRecord, 2);

	readFile(&fixedHeader.numberOfChannels);

	// Load variable header.
	seekFile(2);
	assert(ftell(file) == 256);

	variableHeader.label = new char[getChannelCount()][16 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(variableHeader.label[i], 16);
		variableHeader.label[i][16] = 0;
	}

	variableHeader.typeOfSensor = new char[getChannelCount()][80 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(variableHeader.typeOfSensor[i], 80);
		variableHeader.typeOfSensor[i][80] = 0;
	}

	seekFile(6*getChannelCount());

	variableHeader.physicalDimensionCode = new uint16_t[getChannelCount()];
	readFile(variableHeader.physicalDimensionCode, getChannelCount());

	variableHeader.physicalMinimum = new double[getChannelCount()];
	readFile(variableHeader.physicalMinimum, getChannelCount());

	variableHeader.physicalMaximum = new double[getChannelCount()];
	readFile(variableHeader.physicalMaximum, getChannelCount());

	variableHeader.digitalMinimum = new double[getChannelCount()];
	readFile(variableHeader.digitalMinimum, getChannelCount());

	variableHeader.digitalMaximum = new double[getChannelCount()];
	readFile(variableHeader.digitalMaximum, getChannelCount());

	seekFile(64*getChannelCount());

	variableHeader.timeOffset = new float[getChannelCount()];
	readFile(variableHeader.timeOffset, getChannelCount());

	variableHeader.lowpass = new float[getChannelCount()];
	readFile(variableHeader.lowpass, getChannelCount());

	variableHeader.highpass = new float[getChannelCount()];
	readFile(variableHeader.highpass, getChannelCount());

	variableHeader.notch = new float[getChannelCount()];
	readFile(variableHeader.notch, getChannelCount());

	variableHeader.samplesPerRecord = new uint32_t[getChannelCount()];
	readFile(variableHeader.samplesPerRecord, getChannelCount());

	variableHeader.typeOfData = new uint32_t[getChannelCount()];
	readFile(variableHeader.typeOfData, getChannelCount());

	variableHeader.sensorPosition = new float[getChannelCount()][3];
	readFile(*variableHeader.sensorPosition, 3*getChannelCount());

	variableHeader.sensorInfo = new char[getChannelCount()][20];
	readFile(*variableHeader.sensorInfo, 20*getChannelCount());

	assert(ftell(file) == 256 + 256*getChannelCount());

	// Initialize other members.
	samplesRecorded = getChannelCount()*variableHeader.samplesPerRecord[0]*fixedHeader.numberOfDataRecords;

	startOfData = 256*fixedHeader.headerLength;

#define CASE(a_, b_)\
case a_:\
	dataTypeSize = sizeof(b_);\
	convertSampleToDouble = [] (void* sample) -> double\
	{\
		b_ tmp = *reinterpret_cast<b_*>(sample);\
		return static_cast<double>(tmp);\
	};\
	break;

	switch (variableHeader.typeOfData[0])
	{
		CASE(1, int8_t);
		CASE(2, uint8_t);
		CASE(3, int16_t);
		CASE(4, uint16_t);
		CASE(5, int32_t);
		CASE(6, uint32_t);
		CASE(7, int64_t);
		CASE(8, uint64_t);
		CASE(16, float);
		CASE(17, double);
	default:
		assert(0); // error unsupported type
		break;
	}

	scale = new double[getChannelCount()];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		scale[i] = (variableHeader.digitalMaximum[i] - variableHeader.digitalMinimum[i])/
				   (variableHeader.physicalMaximum[i] - variableHeader.physicalMinimum[i]);
	}

	//samplingFrequency
}

GDF2::~GDF2()
{
	fclose(file);
	delete[] scale;

	// Delete variable header.
	delete[] variableHeader.label;
	delete[] variableHeader.typeOfSensor;
	delete[] variableHeader.physicalDimensionCode;
	delete[] variableHeader.physicalMinimum;
	delete[] variableHeader.physicalMaximum;
	delete[] variableHeader.digitalMinimum;
	delete[] variableHeader.digitalMaximum;
	delete[] variableHeader.timeOffset;
	delete[] variableHeader.lowpass;
	delete[] variableHeader.highpass;
	delete[] variableHeader.notch;
	delete[] variableHeader.samplesPerRecord;
	delete[] variableHeader.typeOfData;
	delete[] variableHeader.sensorPosition;
	delete[] variableHeader.sensorInfo;
}

void GDF2::readData(double* data, unsigned int firstSample, unsigned int lastSample)
{
	// test firstSample <= lastSample
	// test lastSample < samplesRecorded

	int samplesPerRecord = variableHeader.samplesPerRecord[0],
		n = lastSample - firstSample + 1,
		dataIndex = 0,
		recordI = firstSample/samplesPerRecord,
		lastRecordI = lastSample/samplesPerRecord;

	seekFile(startOfData + recordI*samplesPerRecord*getChannelCount()*dataTypeSize, true);

	for (; recordI <= lastRecordI; ++recordI)
	{
		int recordOffset = (firstSample + dataIndex)%samplesPerRecord;
		int samplesToRead = min(samplesPerRecord - recordOffset, n - dataIndex);

		for (unsigned int channelI = 0; channelI < getChannelCount(); ++channelI)
		{
			int skip = recordOffset*dataTypeSize;;
			if (channelI > 0)
			{
				skip += (samplesPerRecord - samplesToRead - recordOffset)*dataTypeSize;
			}

			if (skip > 0)
			{
				seekFile(skip);
			}

			for (int i = 0; i < samplesToRead; ++i)
			{
				double tmp;
				char rawTmp[8];

				fread(rawTmp, dataTypeSize, 1, file);
				changeEndianness(rawTmp, 2);
				changeEndianness(rawTmp, 2);
				tmp = convertSampleToDouble(rawTmp);

				// Calibration.
				tmp -= variableHeader.digitalMinimum[channelI];
				tmp /= scale[channelI];
				tmp += variableHeader.physicalMinimum[channelI];

				data[dataIndex + channelI*n + i] = tmp;
			}
		}

		dataIndex += samplesToRead;
	}
}

template<typename T>
void GDF2::readFile(T* val, int elements)
{
	size_t elementsRead = fread(val, sizeof(T), elements, file);
	if (elementsRead != elements)
	{
		printf("eof=%d error=%d", feof(file), ferror(file));
	}
	assert(elementsRead == elements); // assert and/or exception

	if (isLittleEndian == false)
	{
		for (int i = 0; i < elements; ++i)
		{
			changeEndianness(val + i);
		}
	}
}

void GDF2::seekFile(int position, bool fromStart)
{
	int res = fseek(file, position, fromStart ? SEEK_SET : SEEK_CUR);
	assert(res == 0); // error if res is not zero
}
