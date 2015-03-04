#ifndef DATAFILE_H
#define DATAFILE_H

#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>
#include <utility>

class DataFile
{
public:
	virtual ~DataFile() {}

	virtual double getSamplingFrequency() = 0;
	virtual unsigned int getChannelCount() = 0;
	virtual uint64_t getSamplesRecorded() = 0;
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) = 0;
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) = 0;

	static std::pair<std::int64_t, std::int64_t> getBlockBoundaries(int index, unsigned int blockSize)
	{
		return std::pair<std::int64_t, std::int64_t>(index*blockSize, (index + 1)*blockSize - 1);
	}

protected:
	bool testLittleEndian()
	{
		int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes != 0;
	}
	void changeEndianness(char* data, int n)
	{
		assert(n%2 == 0);

		for (int i = 0, nHalf = n/2; i < nHalf; ++i)
		{
			std::swap(data[i], data[n - i - 1]);
		}
	}
	template<typename T>
	void changeEndianness(T* val)
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}
};

#endif // DATAFILE_H
