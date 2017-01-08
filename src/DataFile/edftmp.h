#include "datafile.h"

#include <cstdio>
#include <mutex>

#ifndef EDFTMP_H
#define EDFTMP_H

class QDateTime;

namespace AlenkaFile
{
class EDF;
}

/**
 * @brief A class implementing the EDF file type.
 *
 * This implementation is only temporary and not complete.
 * It uses a class from an external library which will eventually
 * replace this class.
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
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) override;
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) override;

protected:
	/**
	 * @brief Creates a default montage with an emty event table.
	 * @return True if the loading finished successfully.
	 */
	virtual bool load() override;

private:
	std::mutex fileMutex;
	AlenkaFile::EDF* file;
};

#endif // EDFTMP_H
