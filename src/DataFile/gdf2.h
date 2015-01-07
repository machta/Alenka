#include "datafile.h"

#include <cstdio>

#ifndef GDF2_H
#define GDF2_H

class GDF2 : public DataFile
{
public:
	GDF2(const char* filePath);
	virtual ~GDF2();

	virtual double getFS()
	{
		return Fs;
	}
	virtual unsigned int getChannelCount()
	{
		return channelCount;
	}
	virtual uint64_t getSamplesRecorded()
	{
		return samplesRecorded;
	}
	virtual void readData(double* data, unsigned int firstSample, unsigned int lastSample);

private:
	double Fs;
	unsigned int channelCount;
	uint64_t samplesRecorded;

	FILE* file;
	int startOfData;
	int samplesPerRecord;
	bool isLittleEndian;
	double* physMin;
	double* physMax;
	double* digMin;
	double* digMax;
	double* scale;
	int dataType;

	template<typename T>
	void readFromFile(T* val, int position, int elements = 1);
};

#endif // GDF2_H
