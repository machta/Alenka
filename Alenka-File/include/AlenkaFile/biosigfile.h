#ifndef ALENKAFILE_BIOSIGFILE_H
#define ALENKAFILE_BIOSIGFILE_H

#include "datafile.h"

#include <memory>
#include <vector>

// This is needed because I cannot figure out how to forward declare HDRTYPE.
struct BioSigHeaderWrapper;

// TODO: Add libbiosig-dev to dependencies.
namespace AlenkaFile {

/**
 * @brief This is an adapter class for BioSig library.
 */
class BioSigFile : public DataFile {
  double samplingFrequency;
  int numberOfChannels;
  uint64_t samplesRecorded;
  std::unique_ptr<BioSigHeaderWrapper> fileHeader;
  int readChunk;
  std::vector<double> readChunkBuffer;

public:
  BioSigFile(const std::string &filePath);
  ~BioSigFile() override;

  double getSamplingFrequency() const override { return samplingFrequency; }
  unsigned int getChannelCount() const override { return numberOfChannels; }
  uint64_t getSamplesRecorded() const override { return samplesRecorded; }
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

private:
  template <typename T>
  void readChannelsFloatDouble(std::vector<T *> dataChannels,
                               uint64_t firstSample, uint64_t lastSample);
  void openFile();
  bool checkOpenFile();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_BIOSIGFILE_H
