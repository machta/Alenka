#ifndef DATAFILE_H
#define DATAFILE_H

#include <cstdint>

class DataFile
{
public:
	virtual ~DataFile() {}

	virtual double getSamplingFrequency() = 0;
	virtual unsigned int getChannelCount() = 0;
	virtual uint64_t getSamplesRecorded() = 0;
	virtual void readData(float* data, uint64_t firstSample, uint64_t lastSample) = 0;
	virtual void readData(double* data, uint64_t firstSample, uint64_t lastSample) = 0;

protected:
	bool testLittleEndian()
	{
		int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes != 0;
	}
	void changeEndianness(char* bytes, int n)
	{
		for (int i = 0, nHalf = n/2; i < nHalf; ++i)
		{
			char tmp = bytes[i];
			bytes[i] = bytes[n - i - 1];
			bytes[n - i - 1] = tmp;
		}
	}
	template<typename T>
	void changeEndianness(T* val)
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}
};

#endif // DATAFILE_H
