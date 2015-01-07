#ifndef DATAFILE_H
#define DATAFILE_H

#include <cstdint>

class DataFile
{
public:
	virtual ~DataFile() {}

	virtual double getFS() = 0;
	virtual unsigned int getChannelCount() = 0;
	virtual uint64_t getSamplesRecorded() = 0;
	virtual void readData(double* data, unsigned int firstSample, unsigned int lastSample) = 0;

protected:
	bool testLittleEndian()
	{
		int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes != 0;
	}

	template<typename T>
	void changeEndianness(T* val)
	{
		char* bytes = reinterpret_cast<char*>(val);
		int n = sizeof(T);
		int nHalf = n/2;

		for (int i = 0; i < nHalf; ++i)
		{
			char tmp = bytes[i];
			bytes[i] = bytes[n - i - 1];
			bytes[n - i - 1] = tmp;
		}
	}
};

#endif // DATAFILE_H
