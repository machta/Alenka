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
	assert(file != NULL);// test whether the file exists, if it was opened properly, if it is gdf2

	uint16_t numberOfChannels;
	readFromFile(&numberOfChannels, 252);
	channelCount = numberOfChannels;

	int64_t numberOfDataRecords;
	readFromFile(&numberOfDataRecords, 236);
	assert(numberOfDataRecords != -1); // error if numberOfDataRecords == -1

	// Assume that all channels have the same Fs and dataType.
	uint32_t samplesPerRecord;
	readFromFile(&samplesPerRecord, 256 + 216*numberOfChannels);
	this->samplesPerRecord = samplesPerRecord;

	uint32_t dataType;
	readFromFile(&dataType, 256 + 220*numberOfChannels);
	this->dataType = dataType;

	samplesRecorded = (uint64_t)(numberOfDataRecords)*samplesPerRecord;

	uint32_t num, den; // Duration of a data record.
	readFromFile(&num, 244);
	readFromFile(&den, 244 + 4);
	double duration = (double)(num)/den;

	Fs = samplesPerRecord/duration;

	uint16_t headerLength;
	readFromFile(&headerLength, 184);
	startOfData = headerLength*256;

	physMin = new double[channelCount];
	physMax = new double[channelCount];
	digMin = new double[channelCount];
	digMax = new double[channelCount];

	readFromFile(physMin, 256 + 104*channelCount, channelCount);
	readFromFile(physMax, 256 + 112*channelCount, channelCount);
	readFromFile(digMin, 256 + 120*channelCount, channelCount);
	readFromFile(digMax, 256 + 128*channelCount, channelCount);

	scale = new double[channelCount];
	for (int i = 0; i < channelCount; ++i)
	{
		scale[i] = (digMax[i] - digMin[i])/(physMax[i] - physMin[i]);
	}
}

GDF2::~GDF2()
{
	fclose(file);

	delete[] physMin;
	delete[] physMax;
	delete[] digMin;
	delete[] digMax;
	delete[] scale;
}

#define CASE(_a, _b)\
	case _a:\
		rPosition = startOfData + (recordI*channelCount*samplesPerRecord + channelI*samplesPerRecord + recordOffset + i)*sizeof(_b);\
		readFromFile(&_b, rPosition);\
		tmp = _b;\
		break;

void GDF2::readData(double* data, unsigned int firstSample, unsigned int lastSample)
{
	// test firstSample <= lastSample
	// test lastSample < samplesRecorded

	union
	{
		int8_t	 int8;		// code 1
		uint8_t  uint8;		// ...
		int16_t  int16;
		uint16_t uint16;
		int32_t  int32;
		uint32_t uint32;
		int64_t  int64;
		uint64_t uint64;	// code 8

		float  float32;		// code 16
		double float64;		// code 17
	} sample;

	int n = lastSample - firstSample + 1;
	int dataIndex = 0;

	for (int recordI = firstSample/samplesPerRecord, lastRecordI = lastSample/samplesPerRecord; recordI <= lastRecordI; ++recordI)
	{
		int recordOffset = (firstSample + dataIndex)%samplesPerRecord;
		int samplesToRead = min(samplesPerRecord - recordOffset, n - dataIndex - recordOffset);

		for (int channelI = 0; channelI < channelCount; ++channelI)
		{
			int dPosition = channelI*n + dataIndex;

			for (int i = 0; i < samplesToRead; ++i)
			{
				// Load the sample and convert it to double.
				double tmp;
				int rPosition;

				switch (dataType)
				{
				CASE(1,sample.int8);
				CASE(2,sample.uint8);
				CASE(3,sample.int16);
				CASE(4,sample.uint16);
				CASE(5,sample.int32);
				CASE(6,sample.uint32);
				CASE(7,sample.int64);
				CASE(8,sample.uint64);
				CASE(16,sample.float32);
				CASE(17,sample.float64);
				default:
					assert(0);// error unsupported type
					break;
				}

				//if (i == 0) printf("c=%2d tmp=%10.2lf dP=%5d rP=%5d\n", channelI + 1, tmp, dPosition, rPosition-5376);

				// Calibration.
				tmp -= digMin[channelI];
				tmp /= scale[channelI];
				tmp += physMin[channelI];
				data[dPosition + i] = tmp;
			}
		}

		dataIndex += samplesPerRecord;
	}
}

template<typename T>
void GDF2::readFromFile(T* val, int position, int elements)
{
	int res = fseek(file, position, SEEK_SET);
	assert(res == 0); // error if res is not zero

	size_t elementsRead = fread(val, sizeof(T), elements, file);
	if (elementsRead != elements)
	{
		int dummy = 0;
		printf("eof=%d error=%d position=%d\n", feof(file), ferror(file), position);
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
