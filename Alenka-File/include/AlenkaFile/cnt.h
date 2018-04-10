#ifndef ALENKAFILE_CNT_H
#define ALENKAFILE_CNT_H

#include "datafile.h"

#include <fstream>
#include <memory>
#include <stdint.h>
#include <vector>

// This is needed because I cannot figure out how to forward declare HDRTYPE.
struct CntHeaderWrapper;

// TODO: Add libbiosig-dev to dependencies.
namespace AlenkaFile {

/**
 * @brief This is an adapter class for BioSig library.
 */
class CNT : public DataFile {
  double samplingFrequency;
  int numberOfChannels, dataOffset;
  uint64_t samplesRecorded;
  std::unique_ptr<CntHeaderWrapper> fileHeader;
  std::vector<int16_t> sampleBuffer;
  std::vector<std::pair<float, float>> scalingFactors;
  std::ifstream file;

public:
  CNT(const std::string &filePath);
  ~CNT() override;

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

#endif // ALENKAFILE_CNT_H
