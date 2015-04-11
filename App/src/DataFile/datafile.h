#ifndef DATAFILE_H
#define DATAFILE_H

#include "montagetable.h"
#include "eventtypetable.h"
#include "infotable.h"

#include <cstdint>
#include <utility>
#include <string>
#include <functional>

class QXmlStreamReader;
class QXmlStreamWriter;
class QDateTime;

class DataFile
{
public:
	DataFile(const std::string& filePath);
	virtual ~DataFile();

	virtual double getSamplingFrequency() const = 0;
	virtual unsigned int getChannelCount() const = 0;
	virtual uint64_t getSamplesRecorded() const = 0;
	virtual QDateTime getStartDate() const = 0;
	virtual void save();
	MontageTable* getMontageTable()
	{
		return &montageTable;
	}
	EventTypeTable* getEventTypeTable()
	{
		return &eventTypeTable;
	}
	std::string getFilePath() const
	{
		return filePath;
	}
	InfoTable* getInfoTable()
	{
		return &infoTable;
	}
	QDateTime sampleToDate(int sample);
	QDateTime sampleToOffset(int sample);
	QString sampleToDateTimeString(int sample);
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) = 0;
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) = 0;

	static std::pair<std::int64_t, std::int64_t> blockIndexToSampleRange(int index, unsigned int blockSize)
	{
		using namespace std;

		int64_t from = index*blockSize,
				to = (index + 1)*blockSize - 1;

		from -= index;
		to -= index;

		return make_pair(from, to);
	}
	static std::pair<std::int64_t, std::int64_t> sampleRangeToBlockRange(std::pair<std::int64_t, std::int64_t> range, unsigned int blockSize)
	{
		using namespace std;

		int64_t from = range.first,
				to = range.second;

		from /= blockSize - 1;
		to = (to - 1)/(blockSize - 1);

		return make_pair(from, to);
	}

protected:
	virtual bool load();
	bool testLittleEndian() const
	{
		unsigned int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes == 1;
	}
	void changeEndianness(char* data, int size) const
	{
		for (int i = 0, sizeHalf = size/2; i < sizeHalf; ++i)
		{
			std::swap(data[i], data[size - i - 1]);
		}
	}
	template<typename T>
	void changeEndianness(T* val) const
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}

private:
	std::string filePath;
	EventTypeTable eventTypeTable;
	MontageTable montageTable;
	InfoTable infoTable;

	bool loadXMLFile(const std::string& extension, std::function<void (QXmlStreamReader*)> loadFunction);
	void saveXMLFile(const std::string& extension, std::function<void (QXmlStreamWriter*)> saveFunction);
};

#endif // DATAFILE_H
