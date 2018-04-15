#ifndef ALENKAFILE_DATAFILE_H
#define ALENKAFILE_DATAFILE_H

#include "abstractdatamodel.h"

#include <cassert>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

// TODO: Add a public interface for access to the headers.
// TODO: Get rid of the unsigned types in the interface.

namespace AlenkaFile {

/**
 * @brief An abstract base class for the data files.
 *
 * DataFile implements all the operations concerning the secondary files --
 * extensions of the files containing the actual signal data. It is assumed that
 * the primary and secondary file is always in the same directory, and the
 * secondary file's name is primaryFileName.info (e.g. primary file is
 * 'sample.gdf' and secondary is 'sample.gdf.info', or 'sample.gdf.mont'.
 *
 * To implement a new file type you need to subclass DataFile and implement
 * all the pure virtual functions to provide the implementation of the most
 * basic file-type specific functionality.
 *
 * It is assumed that every channel has the same sampling frequency and length
 * (the same total number of samples recorded).
 */
class DataFile {
  std::string filePath;
  DataModel *dataModel;
  std::vector<double> physicalMax, physicalMin;

public:
  /**
   * @brief DataFile constructor.
   * @param filePath The file path of the primary file.
   */
  DataFile(const std::string &filePath) : filePath(filePath) {}
  virtual ~DataFile() = default;

  std::string getFilePath() const { return filePath; }

  /**
   * @brief Returns the sampling frequency of the stored signal.
   */
  virtual double getSamplingFrequency() const = 0;

  /**
   * @brief Returns the number of channels of the stored signal.
   */
  virtual unsigned int getChannelCount() const = 0;

  /**
   * @brief Return the total number of samples recorded for each channel.
   */
  virtual uint64_t getSamplesRecorded() const = 0;

  /**
   * @brief Returns days since year 0.
   *
   * @note Works the same as datenum() in Matlab.
   *
   * @return Invalid time means you should use getStandardStartDate() instead.
   */
  virtual double getStartDate() const { return INVALID_DATE; }
  // TODO: Add more date unit-tests.

  /**
   * @brief An alternative date implementation. Use this only if getStartDate()
   * doesn't work.
   *
   * @return The default is meaningless -- return the current time.
   */
  virtual std::time_t getStandardStartDate() const {
    return std::time(nullptr);
  }

  /**
   * @brief Saves the .info file.
   *
   * @param infoFile [out]
   */
  void saveSecondaryFile(std::string montFilePath = "");
  virtual void save() { saveSecondaryFile(); }

  /**
   * @brief Loads the information from the .info file.
   * @return True if the .info file was located.
   *
   * This method tries to load the necessary information from the .info file.
   * If the file is not located, false is returned. The extending class can then
   * load this information instead from the primary file.
   *
   * An empty montage is always created.???
   * @todo Make sure load can be called repeatedly with the same results.
   *
   * @param infoFile [in]
   */
  bool loadSecondaryFile(std::string montFilePath = "");
  virtual bool load() { return loadSecondaryFile(); }

  /**
   * @brief Reads signal data specified by the sample range.
   *
   * The range can exceed the boundaries of the signal data stored in the file.
   * These extra samples are set to zero.
   *
   * The actual reading from the primary file is delegated to readChannels.
   * @param data [out] The signal data is copied to this buffer.
   * @param firstSample The first sample to be read.
   * @param lastSample The last sample to be read.
   */
  void readSignal(float *data, int64_t firstSample, int64_t lastSample);

  /**
   * \overload void readSignal(float* data, int64_t firstSample, int64_t
   * lastSample)
   */
  void readSignal(double *data, int64_t firstSample, int64_t lastSample);

  /**
   * @brief Reads signal data specified by the sample range.
   *
   * The range cannot exceed the boundaries of the signal.
   * @param dataChannels A vector of pointers to the beginning of channels.
   * @param firstSample The first sample to be read.
   * @param lastSample The last sample to be read.
   */
  virtual void readChannels(std::vector<float *> dataChannels,
                            uint64_t firstSample, uint64_t lastSample) = 0;

  /**
   * \overload virtual void readChannels(std::vector<double*> dataChannels,
   * uint64_t firstSample, uint64_t lastSample) = 0
   */
  virtual void readChannels(std::vector<double *> dataChannels,
                            uint64_t firstSample, uint64_t lastSample) = 0;

  DataModel *getDataModel() const {
    assert(dataModel->montageTable() && dataModel->eventTypeTable());
    return dataModel;
  }
  void setDataModel(DataModel *dataModel) { this->dataModel = dataModel; }

  virtual double getPhysicalMaximum(unsigned int channel);
  virtual double getPhysicalMinimum(unsigned int channel);
  virtual double getDigitalMaximum(unsigned int /*channel*/) { return 32767; }
  virtual double getDigitalMinimum(unsigned int /*channel*/) { return -32768; }
  virtual std::string getLabel(unsigned int channel) = 0;

  /**
   * @brief Get labels for all channels at once.
   * @return A vector of labels that are whitespace-trimmed.
   */
  std::vector<std::string> getLabels();

  /**
   * @brief Initialize the montage at index with the default values.
   */
  virtual void fillDefaultMontage(int index);

  static const int daysUpTo1970 = 719529; // datenum('01-Jan-1970')
  static double INVALID_DATE;

  /**
   * @brief Tests endianness.
   * @return Returns true if this computer is little-endian, false otherwise.
   */
  static bool testLittleEndian() {
    unsigned int number = 1;
    char *bytes = reinterpret_cast<char *>(&number);
    return *bytes == 1;
  }

  /**
   * @brief Reverse endianness of data.
   * @param data [in,out]
   * @param size Size in bytes.
   */
  static void changeEndianness(char *data, int size) {
    for (int i = 0, sizeHalf = size / 2; i < sizeHalf; ++i)
      std::swap(data[i], data[size - i - 1]);
  }

  /**
   * @brief Reverse endianness of val.
   * @param val [in,out]
   */
  template <typename T> static void changeEndianness(T *val) {
    changeEndianness(reinterpret_cast<char *>(val), sizeof(T));
  }

private:
  void computePhysicalMinMax();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAFILE_H
