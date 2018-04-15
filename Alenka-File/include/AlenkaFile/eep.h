#ifndef EEP_H
#define EEP_H

#include "datafile.h"

#include <vector>

typedef int cntfile_t;

namespace AlenkaFile {

/**
 * @brief Uses libEEP to implement ANT CNT file format.
 */
class EEP : public DataFile {
  double samplingFrequency;
  int numberOfChannels;
  uint64_t samplesRecorded;
  cntfile_t fileHandle = -1;

public:
  EEP(const std::string &filePath);
  ~EEP() override;

  double getSamplingFrequency() const override { return samplingFrequency; }
  unsigned int getChannelCount() const override { return numberOfChannels; }
  uint64_t getSamplesRecorded() const override { return samplesRecorded; }
  std::time_t getStandardStartDate() const override;
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

  //  double getPhysicalMaximum(unsigned int channel) override;
  //  double getPhysicalMinimum(unsigned int channel) override;
  //  double getDigitalMaximum(unsigned int channel) override;
  //  double getDigitalMinimum(unsigned int channel) override;
  std::string getLabel(unsigned int channel) override;

private:
  template <typename T>
  void readChannelsFloatDouble(std::vector<T *> dataChannels,
                               uint64_t firstSample, uint64_t lastSample);
  void openFile();
};

} // namespace AlenkaFile

#endif // EEP_H
