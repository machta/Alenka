#ifndef DATAFILE_H
#define DATAFILE_H

#include "montagetable.h"
#include "eventtypetable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>

class DataFile
{
public:
	DataFile(const std::string& filePath);
	virtual ~DataFile();

	virtual double getSamplingFrequency() const = 0;
	virtual unsigned int getChannelCount() const = 0;
	virtual uint64_t getSamplesRecorded() const = 0;
	virtual void save();
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) = 0;
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) = 0;
	EventTypeTable* getEventTypeTable()
	{
		return &eventTypeTable;
	}
	std::vector<MontageTable*> getMontageTables()
	{
		return montageTables;
	}

	static std::pair<std::int64_t, std::int64_t> getBlockBoundaries(int index, unsigned int blockSize)
	{
		using namespace std;

		int64_t from = index*blockSize,
				to = (index + 1)*blockSize - 1;

		if (index > 0)
		{
			--from;
			--to;
		}

		return make_pair(from, to);
	}

protected:
	std::string filePath;
	std::vector<MontageTable*> montageTables;
	EventTypeTable eventTypeTable;

	bool testLittleEndian() const
	{
		unsigned int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes == 1;
	}
	void changeEndianness(char* data, int n) const
	{
		assert(n%2 == 0);

		for (int i = 0, nHalf = n/2; i < nHalf; ++i)
		{
			std::swap(data[i], data[n - i - 1]);
		}
	}
	template<typename T>
	void changeEndianness(T* val) const
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}
};

#endif // DATAFILE_H
