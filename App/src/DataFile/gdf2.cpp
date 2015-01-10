#include "gdf2.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <sstream>
#include <exception>

using namespace std;

GDF2::GDF2(const char* filePath)
{
	isLittleEndian = testLittleEndian();

	file = fopen(filePath, "r+b");
	if (file == nullptr)
	{
		stringstream ss;
		ss << "File '" << filePath << "' not found.";
		throw runtime_error(ss.str());
	}

	// Load fixed header.
	seekFile(0, true);

	readFile(fh.versionID, 8);
	fh.versionID[8] = 0;
	int major, minor;
	sscanf(fh.versionID + 4, "%d.%d", &major, &minor);
	version = minor + 100*major;

	if (string(fh.versionID, 3) != "GDF" || major != 2)
	{
		throw runtime_error("Unrecognized file format.");
	}

	readFile(fh.patientID, 66);
	fh.patientID[66] = 0;

	seekFile(10);

	readFile(&fh.drugs);

	readFile(&fh.weight);

	readFile(&fh.height);

	readFile(&fh.patientDetails);

	readFile(fh.recordingID, 64);
	fh.recordingID[64] = 0;

	readFile(fh.recordingLocation, 4);

	readFile(fh.startDate, 2);

	readFile(fh.birthday, 2);

	readFile(&fh.headerLength);

	readFile(fh.ICD, 6);

	readFile(&fh.equipmentProviderID);

	seekFile(6);

	readFile(fh.headsize, 3);

	readFile(fh.positionRE, 3);

	readFile(fh.positionGE, 3);

	readFile(&fh.numberOfDataRecords);

	double duration;
	if (version > 220)
	{
		double* ptr = reinterpret_cast<double*>(fh.durationOfDataRecord);
		readFile(ptr, 1);
		duration = *ptr;
	}
	else
	{
		uint32_t* ptr = reinterpret_cast<uint32_t*>(fh.durationOfDataRecord);
		readFile(ptr, 2);
		duration = static_cast<double>(ptr[0])/static_cast<double>(ptr[1]);
	}

	readFile(&fh.numberOfChannels);

	// Load variable header.
	seekFile(2);
	assert(ftell(file) == 256);

	vh.label = new char[getChannelCount()][16 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.label[i], 16);
		vh.label[i][16] = 0;
	}

	vh.typeOfSensor = new char[getChannelCount()][80 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.typeOfSensor[i], 80);
		vh.typeOfSensor[i][80] = 0;
	}

	seekFile(6*getChannelCount());

	vh.physicalDimensionCode = new uint16_t[getChannelCount()];
	readFile(vh.physicalDimensionCode, getChannelCount());

	vh.physicalMinimum = new double[getChannelCount()];
	readFile(vh.physicalMinimum, getChannelCount());

	vh.physicalMaximum = new double[getChannelCount()];
	readFile(vh.physicalMaximum, getChannelCount());

	vh.digitalMinimum = new double[getChannelCount()];
	readFile(vh.digitalMinimum, getChannelCount());

	vh.digitalMaximum = new double[getChannelCount()];
	readFile(vh.digitalMaximum, getChannelCount());

	seekFile(64*getChannelCount());

	vh.timeOffset = new float[getChannelCount()];
	readFile(vh.timeOffset, getChannelCount());

	vh.lowpass = new float[getChannelCount()];
	readFile(vh.lowpass, getChannelCount());

	vh.highpass = new float[getChannelCount()];
	readFile(vh.highpass, getChannelCount());

	vh.notch = new float[getChannelCount()];
	readFile(vh.notch, getChannelCount());

	vh.samplesPerRecord = new uint32_t[getChannelCount()];
	readFile(vh.samplesPerRecord, getChannelCount());

	vh.typeOfData = new uint32_t[getChannelCount()];
	readFile(vh.typeOfData, getChannelCount());

	vh.sensorPosition = new float[getChannelCount()][3];
	readFile(*vh.sensorPosition, 3*getChannelCount());

	vh.sensorInfo = new char[getChannelCount()][20];
	readFile(*vh.sensorInfo, 20*getChannelCount());

	assert(ftell(file) == 256 + 256*getChannelCount());

	// Initialize other members.
	samplesRecorded = vh.samplesPerRecord[0]*fh.numberOfDataRecords;

	startOfData = 256*fh.headerLength;

#define CASE(a_, b_)\
case a_:\
	dataTypeSize = sizeof(b_);\
	convertSampleToDouble = [] (void* sample) -> double\
	{\
		b_ tmp = *reinterpret_cast<b_*>(sample);\
		return static_cast<double>(tmp);\
	};\
	convertSampleToFloat = [] (void* sample) -> float\
	{\
		b_ tmp = *reinterpret_cast<b_*>(sample);\
		return static_cast<float>(tmp);\
	};\
	break;

	switch (vh.typeOfData[0])
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
		throw runtime_error("Unsupported data type.");
		break;
	}

	scale = new double[getChannelCount()];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		scale[i] = (vh.digitalMaximum[i] - vh.digitalMinimum[i])/
				   (vh.physicalMaximum[i] - vh.physicalMinimum[i]);
	}

	samplingFrequency = vh.samplesPerRecord[0]/duration;
}

GDF2::~GDF2()
{
	fclose(file);
	delete[] scale;

	// Delete variable header.
	delete[] vh.label;
	delete[] vh.typeOfSensor;
	delete[] vh.physicalDimensionCode;
	delete[] vh.physicalMinimum;
	delete[] vh.physicalMaximum;
	delete[] vh.digitalMinimum;
	delete[] vh.digitalMaximum;
	delete[] vh.timeOffset;
	delete[] vh.lowpass;
	delete[] vh.highpass;
	delete[] vh.notch;
	delete[] vh.samplesPerRecord;
	delete[] vh.typeOfData;
	delete[] vh.sensorPosition;
	delete[] vh.sensorInfo;
}

template<typename T>
void GDF2::readDataLocal(T* data, int64_t firstSample, int64_t lastSample)
{
	if (lastSample < firstSample)
	{
		throw invalid_argument("lastSample must be greater or equeal than firstSample.");
	}

	int samplesPerRecord = vh.samplesPerRecord[0];
	uint64_t rowLen = lastSample - firstSample + 1,
			 dataOffset, dataIndex = 0, recordI, lastRecordI, n;

	if (firstSample < 0)
	{
		dataOffset = -firstSample;
		firstSample = recordI = 0;

		for (int j = 0; j < getChannelCount(); ++j)
		{
			for (int i = 0; i < dataOffset; ++i)
			{
				data[j*rowLen + i] = 0;
			}
		}
	}
	else
	{
		dataOffset = 0;
		recordI = firstSample/samplesPerRecord;
	}

	if (lastSample >= samplesRecorded)
	{
		int extra = lastSample - samplesRecorded + 1;
		lastSample = samplesRecorded - 1;
		lastRecordI = lastSample/samplesPerRecord;
		n = lastSample - firstSample + 1;

		for (int j = 0; j < getChannelCount(); ++j)
		{
			for (int i = 0; i < extra; ++i)
			{
				data[j*rowLen + dataOffset + n + i] = 0;
			}
		}
	}
	else
	{
		n = lastSample - firstSample + 1;
		lastRecordI = lastSample/samplesPerRecord;
	}

	seekFile(startOfData + recordI*samplesPerRecord*getChannelCount()*dataTypeSize, true);

	for (; recordI <= lastRecordI; ++recordI)
	{
		int recordOffset = static_cast<int>((firstSample + dataIndex)%samplesPerRecord);
		int samplesToRead = static_cast<int>(min(static_cast<uint64_t>(samplesPerRecord - recordOffset), n - dataIndex));

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
				T tmp;
				char rawTmp[8];

				//fread(rawTmp, dataTypeSize, 1, file);
				readFile(rawTmp, dataTypeSize);

				if (isLittleEndian == false)
				{
					changeEndianness(rawTmp, 2);
				}

				if (is_same<T, float>::value)
				{
					tmp = convertSampleToFloat(rawTmp);
				}
				else
				{
					tmp = convertSampleToDouble(rawTmp);
				}

				// Calibration.
				tmp -= static_cast<T>(vh.digitalMinimum[channelI]);
				tmp /= static_cast<T>(scale[channelI]);
				tmp += static_cast<T>(vh.physicalMinimum[channelI]);

				data[channelI*rowLen + dataOffset + dataIndex + i] = tmp;
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
		stringstream ss;
		if (feof(file))
		{
			ss << "EOF reached prematurely.";
		}
		else
		{
			assert(ferror(file));
			ss << "Error while reading data from file.";
		}
		throw runtime_error(ss.str());
	}

	if (isLittleEndian == false)
	{
		for (int i = 0; i < elements; ++i)
		{
			changeEndianness(val + i);
		}
	}
}

void GDF2::seekFile(size_t position, bool fromStart)
{
	size_t res = fseek(file, static_cast<long>(position), fromStart ? SEEK_SET : SEEK_CUR);
	if (res != 0)
	{
		stringstream ss;
		ss << "seekFile(" << position << ", " << fromStart << ") failed.";
		throw runtime_error(ss.str());
	}
}



