#ifndef ALENKAFILE_EDF_H
#define ALENKAFILE_EDF_H

#include "datafile.h"

#include <memory>
#include <vector>

struct edf_hdr_struct;

namespace AlenkaFile {

/**
 * @brief A class implementing the EDF+ and BDF+ types.
 *
 * There is a limit on the channel count (512) due to the limitations
 * of the EDFlib library.
 */
class EDF : public DataFile {
  double samplingFrequency;
  int numberOfChannels;
  uint64_t samplesRecorded;
  std::unique_ptr<edf_hdr_struct> edfhdr;
  int readChunk;
  std::vector<double> readChunkBuffer;

public:
  /**
   * @brief Constructor.
   * @param filePath The file path of the primary data file.
   */
  EDF(const std::string &filePath);
  ~EDF() override;

  double getSamplingFrequency() const override { return samplingFrequency; }
  unsigned int getChannelCount() const override { return numberOfChannels; }
  uint64_t getSamplesRecorded() const override { return samplesRecorded; }
  double getStartDate() const override;
  void save() override;
  bool load() override;
  void readChannels(std::vector<float *> dataChannels, uint64_t firstSample,
                    uint64_t lastSample) override {
    readChannelsFloatDouble(dataChannels, firstSample, lastSample);
  }
  void readChannels(std::vector<double *> dataChannels, uint64_t firstSample,
                    uint64_t lastSample) override {
    readChannelsFloatDouble(dataChannels, firstSample, lastSample);
  }

  double getPhysicalMaximum(unsigned int channel) override;
  double getPhysicalMinimum(unsigned int channel) override;
  double getDigitalMaximum(unsigned int channel) override;
  double getDigitalMinimum(unsigned int channel) override;
  std::string getLabel(unsigned int channel) override;

  static void saveAs(const std::string &filePath, DataFile *sourceFile);

private:
  template <typename T>
  void readChannelsFloatDouble(std::vector<T *> dataChannels,
                               uint64_t firstSample, uint64_t lastSample);
  void openFile();
  void loadEvents();
  void addUsedEventTypes();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_EDF_H
