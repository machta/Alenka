#ifndef ALENKASIGNAL_SPIKEDET_H
#define ALENKASIGNAL_SPIKEDET_H

#include <atomic>
#include <memory>
#include <vector>

#include "../../spikedet/src/CResultsModel.h"
#include "../../spikedet/src/CSettingsModel.h"
#include "../../spikedet/src/Definitions.h"
#include "../../spikedet/src/spikedetoutput.h"
#include <AlenkaFile/datafile.h>

class CSpikeDetector;

namespace AlenkaSignal {

template <class T> class AbstractSpikedetLoader {
public:
  virtual ~AbstractSpikedetLoader() = default;

  virtual void readSignal(T *data, int64_t firstSample, int64_t lastSample) = 0;
  virtual int64_t sampleCount() = 0;
  virtual int channelCount() = 0;
};

// TODO: Move this class to its own header.
template <class T> class FileSpikedetLoader : public AbstractSpikedetLoader<T> {
  AlenkaFile::DataFile *file;

public:
  FileSpikedetLoader(AlenkaFile::DataFile *file) : file(file) {}

  void readSignal(T *data, int64_t firstSample, int64_t lastSample) override {
    file->readSignal(data, firstSample, lastSample);
  }

  int64_t sampleCount() override { return file->getSamplesRecorded(); }

  int channelCount() override { return file->getChannelCount(); }
};

template <class T>
class VectorSpikedetLoader : public AbstractSpikedetLoader<T> {
  std::vector<T> signal;
  int channels;
  int length;

public:
  VectorSpikedetLoader(std::vector<T> signal, int channels)
      : signal(signal), channels(channels), length(signal.size() / channels) {
    assert(signal.size() == channels * length);
  }

  void readSignal(T *data, int64_t firstSample, int64_t lastSample) override {
    int64_t len = lastSample - firstSample + 1;

    for (int j = 0; j < channels; ++j) {
      for (int64_t i = firstSample; i <= lastSample; ++i) {
        T sample = i < 0 || i >= length ? 0 : signal.at(j * length + i);

        data[j * len + i - firstSample] = sample;
      }
    }
  }

  int64_t sampleCount() override { return length; }

  int channelCount() override { return channels; }
};

class Spikedet {
  const int fs;
  DETECTOR_SETTINGS settings;
  std::atomic<int> progressCurrent;
  std::unique_ptr<CSpikeDetector> detector;

public:
  Spikedet(int fs, bool original, DETECTOR_SETTINGS settings);

  void runAnalysis(AbstractSpikedetLoader<SIGNALTYPE> *loader,
                   CDetectorOutput *out, CDischarges *discharges);

  /**
   * @brief This is used to query the completion status of the analysis.
   *
   * This method is thread-safe. It can be called from a thread other than the
   * one that launched the detector via runAnalysis.
   *
   * @return Returns the percentage towards completion of the operation.
   */
  int progressPercentage() const { return progressCurrent; }

  /**
   * @brief Use cancel to tell the detector to quit the computation at the
   * earliest oppurtunity.
   *
   * Also thread-safe.
   */
  void cancel();

  static DETECTOR_SETTINGS defaultSettings() {
    return DETECTOR_SETTINGS(10, 60, 3.65, 3.65, 0, 5, 4, 300, 50, 0.005, 0.12,
                             200);
  }
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_SPIKEDET_H
