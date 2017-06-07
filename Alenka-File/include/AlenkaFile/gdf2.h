#ifndef ALENKAFILE_GDF2_H
#define ALENKAFILE_GDF2_H

#include "datafile.h"

#include <cmath>
#include <fstream>

namespace AlenkaFile {

/**
 * @brief A class implementing the GDF v2.51 file type.
 *
 * This is my own implementation that doesn't depend on anything but the
 * standard library.
 */
class GDF2 : public DataFile {
public:
  /**
   * @brief GDF2 constructor.
   * @param filePath The file path of the primary data file.
   */
  GDF2(const std::string &filePath, bool uncalibrated = false);
  ~GDF2() override;

  double getSamplingFrequency() const override { return samplingFrequency; }
  unsigned int getChannelCount() const override { return fh.numberOfChannels; }
  uint64_t getSamplesRecorded() const override { return samplesRecorded; }
  double getStartDate() const override {
    double fractionOfDay = ldexp(static_cast<double>(fh.startDate[0]), -32);
    double days = fh.startDate[1];
    return days + fractionOfDay;
  }
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

  double getPhysicalMaximum(unsigned int channel) override {
    if (channel < getChannelCount())
      return vh.physicalMaximum[channel];
    return 0;
  }
  double getPhysicalMinimum(unsigned int channel) override {
    if (channel < getChannelCount())
      return vh.physicalMinimum[channel];
    return 0;
  }
  double getDigitalMaximum(unsigned int channel) override {
    if (channel < getChannelCount())
      return vh.digitalMaximum[channel];
    return 0;
  }
  double getDigitalMinimum(unsigned int channel) override {
    if (channel < getChannelCount())
      return vh.digitalMinimum[channel];
    return 0;
  }
  std::string getLabel(unsigned int channel) override {
    if (channel < getChannelCount())
      return vh.label[channel];
    return nullptr;
  }

private:
  std::fstream file;
  double samplingFrequency;
  uint64_t samplesRecorded;
  int64_t startOfData;
  int64_t startOfEventTable;
  double *scale;
  int dataTypeSize;
  int version;
  char *recordRawBuffer;
  double *recordDoubleBuffer;
  int dataType;

  /**
   * @brief A structure for storing values from the file's fixed header
   * as C++ types at one place.
   */
  struct {
    char versionID[8 + 1];
    char patientID[66 + 1];
    // 10B reserved
    char drugs;
    uint8_t weight;
    uint8_t height;
    char patientDetails;
    char recordingID[64 + 1];
    uint32_t recordingLocation[4];
    uint32_t startDate[2];
    uint32_t birthday[2];
    uint16_t headerLength;
    char ICD[6];
    uint64_t equipmentProviderID;
    // 6B reserved
    uint16_t headsize[3];
    float positionRE[3];
    float positionGE[3];
    int64_t numberOfDataRecords;
    char durationOfDataRecord[8];
    uint16_t numberOfChannels;
    // 2B reserved
  } fh;

  /**
   * @brief A structure for storing values from the file's variable header
   * as C++ types at one place.
   */
  struct {
    char (*label)[16 + 1];
    char (*typeOfSensor)[80 + 1];
    // physicalDimension obsolete
    uint16_t *physicalDimensionCode;
    double *physicalMinimum;
    double *physicalMaximum;
    double *digitalMinimum;
    double *digitalMaximum;
    // prefiltering obsolete
    float *timeOffset;
    float *lowpass;
    float *highpass;
    float *notch;
    uint32_t *samplesPerRecord;
    uint32_t *typeOfData;
    float (*sensorPosition)[3];
    char (*sensorInfo)[20];
  } vh;

  template <typename T>
  void readChannelsFloatDouble(std::vector<T *> dataChannels,
                               const uint64_t firstSample,
                               const uint64_t lastSample);
  void readGdfEventTable();
  void fillDefaultMontage();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_GDF2_H
