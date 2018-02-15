#ifndef ALENKAFILE_MAT_H
#define ALENKAFILE_MAT_H

#include "datafile.h"

#include <string>
#include <vector>

typedef struct _mat_t mat_t;
struct matvar_t;

namespace AlenkaFile {

struct MATvars {
  std::vector<std::string> data{"d"};

  std::string frequency = "fs", multipliers = "mults", date = "date",
              label = "header.label", eventPosition = "out.pos",
              eventDuration = "out.dur", eventChannel = "out.chan";
};

/**
 * @brief This class implements Mat files (a format native to Matlab).
 */
class MAT : public DataFile {
  const int MAX_CHANNELS = 10 * 1000;
  MATvars vars;
  std::vector<mat_t *> files;
  double samplingFrequency;
  int numberOfChannels;
  uint64_t samplesRecorded;
  std::vector<char> tmpBuffer;
  std::vector<matvar_t *> data;
  std::vector<matvar_t *> dataToFree;
  std::vector<unsigned int> dataFileIndex;
  std::vector<int> sizes;
  std::vector<double> multipliers;
  double days = daysUpTo1970;
  std::vector<std::string> labels;

public:
  MAT(const std::string &filePath, MATvars vars = MATvars());
  MAT(const std::vector<std::string> &filePaths, MATvars vars = MATvars());
  ~MAT() override;

  double getSamplingFrequency() const override { return samplingFrequency; }
  unsigned int getChannelCount() const override { return numberOfChannels; }
  uint64_t getSamplesRecorded() const override { return samplesRecorded; }
  double getStartDate() const override { return days; }
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
  std::string getLabel(unsigned int channel) override {
    return labels[channel];
  }

private:
  void openMatFile(const std::string &filePath);
  void construct();
  void readSamplingRate();
  void readData();
  void readMults();
  void readDate();
  void readLabels();
  void readEvents(std::vector<int> *eventPositions,
                  std::vector<int> *eventDurations,
                  std::vector<int> *eventChannels);

  template <typename T>
  void readChannelsFloatDouble(std::vector<T *> dataChannels,
                               uint64_t firstSample, uint64_t lastSample);

  void loadEvents();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_MAT_H
