#include "datafile.h"

#include "../error.h"

#include <cstdio>
#include <mutex>
#include <functional>

#ifndef EDFTMP_H
#define EDFTMP_H

class QDateTime;
class QFile;

namespace AlenkaFile
{
class EDF;
}

/**
 * @brief A class implementing the EDF file type.
 *
 * All methods accessing the information stored in the file are thread-safe.
 */
class EdfTmp : public DataFile
{
public:
	/**
	 * @brief EdfTmp constructor.
	 * @param filePath The file path of the data file without the extension.
	 */
	EdfTmp(const std::string& filePath);
	virtual ~EdfTmp();

	virtual double getSamplingFrequency() const override;
	virtual unsigned int getChannelCount() const override;
	virtual uint64_t getSamplesRecorded() const override;
	virtual QDateTime getStartDate() const override;
	virtual void save() override;
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) override
	{
		readDataLocal(data, firstSample, lastSample);
	}
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) override
	{
		readDataLocal(data, firstSample, lastSample);
	}

protected:
	/**
	 * @brief Creates a default montage with EventTable and TrackTable created
	 * from the information retrieved from the GDF file.
	 * @return True if the loading finished successfully.
	 */
	virtual bool load() override;

private:
	std::mutex fileMutex;
	AlenkaFile::EDF* file;

	double samplingFrequency;
	uint64_t samplesRecorded;
	int64_t startOfData;
	int64_t startOfEventTable;
	bool isLittleEndian;
	double* scale;


	template<typename T>
	void readDataLocal(std::vector<T>* data, int64_t firstSample, int64_t lastSample);
};

#endif // EDFTMP_H
