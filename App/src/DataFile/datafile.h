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

/**
 * @brief An abstract base class of the data files.
 *
 * DataFile implements the operations concerning .mont and .event files that
 * are common for all data file types.
 *
 * To implement a new file type you need to subclass this type and implement
 * the pure virtual functions.
 */
class DataFile
{
public:
	/**
	 * @brief DataFile constructor.
	 * @param filePath The file path of the data file without the extension.
	 */
	DataFile(const std::string& filePath);
	virtual ~DataFile();

	/**
	 * @brief Returns the sampling frequency of the stored signal.
	 */
	virtual double getSamplingFrequency() const = 0;

	/**
	 * @brief Returns the number of channels of the stored signal.
	 */
	virtual unsigned int getChannelCount() const = 0;

	/**
	 * @brief Return the total number of samples recorded for one channel.
	 */
	virtual uint64_t getSamplesRecorded() const = 0;

	/**
	 * @brief Returns the date of the start of recording.
	 */
	virtual QDateTime getStartDate() const = 0;

	/**
	 * @brief Saves the .mont file.
	 */
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

	/**
	 * @brief Returns the date corresponding to the sample.
	 * @param sample Index of the signal sample.
	 */
	QDateTime sampleToDate(int sample);

	/**
	 * @brief sampleToOffset
	 * @param sample Index of the signal sample.
	 */
	QDateTime sampleToOffset(int sample);

	/**
	 * @brief Returns a string representation of the date corresponding to the sample.
	 * @param sample Index of the signal sample.
	 * @param mode Controls the style of the output string.
	 */
	QString sampleToDateTimeString(int sample, InfoTable::TimeMode mode = InfoTable::TimeMode::size);

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range can exceed the boundaries of the signal data stored in the file.
	 * These extra samples are set to zero.
	 * @param data [out] The signal data is stored in this vector.
	 * @param firstSample The first sample to be loaded.
	 * @param lastSample The last sample to be loaded.
	 */
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) = 0;

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range can exceed the boundaries of the signal data stored in the file.
	 * These extra samples are set to zero.
	 * @param data [out] The signal data is stored in this vector.
	 * @param firstSample The first sample to be loaded.
	 * @param lastSample The last sample to be loaded.
	 */
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) = 0;

	/**
	 * @brief Returns the range of samples corresponding to the block index.
	 *
	 * This method ensures that adjacent blocks overlap by one sample.
	 * @param index Index of the block.
	 * @param blockSize The size of the blocks constant for all blocks.
	 */
	static std::pair<std::int64_t, std::int64_t> blockIndexToSampleRange(int index, unsigned int blockSize)
	{
		using namespace std;

		int64_t from = index*blockSize,
				to = (index + 1)*blockSize - 1;

		from -= index;
		to -= index;

		return make_pair(from, to);
	}

	/**
	 * @brief A convenience function for resolving blocks needed to cover a sample range.
	 * @param range The sample range.
	 * @param blockSize The size of the blocks constant for all blocks.
	 */
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
	/**
	 * @brief Loads the info from the .mont file.
	 *
	 * This method tries to load the necessary information from the .mont file.
	 * If the file is not located false is returned to notify the the child
	 * classes that they can load this information instead.
	 * @return Return true if the .mont file was located.
	 */
	virtual bool load();

	/**
	 * @brief Tests endianness.
	 * @return Returns true if this computer is little-endian, false otherwise.
	 */
	bool testLittleEndian() const
	{
		unsigned int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes == 1;
	}

	/**
	 * @brief Reverse endianness.
	 * @param data [in,out]
	 * @param size Size in bytes.
	 */
	void changeEndianness(char* data, int size) const
	{
		for (int i = 0, sizeHalf = size/2; i < sizeHalf; ++i)
		{
			std::swap(data[i], data[size - i - 1]);
		}
	}

	/**
	 * @brief Reverse endianness.
	 * @param val [in,out]
	 */
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

	/**
	 * @brief A convenience method for loading a XML file.
	 * @param extension The extension that is appended to filePath.
	 * @return Return true if the file was found.
	 */
	bool loadXMLFile(const std::string& extension, std::function<void (QXmlStreamReader*)> loadFunction);

	/**
	 * @brief A convenience method for loading a XML file.
	 * @param extension The extension that is appended to filePath.
	 */
	void saveXMLFile(const std::string& extension, std::function<void (QXmlStreamWriter*)> saveFunction);
};

#endif // DATAFILE_H
