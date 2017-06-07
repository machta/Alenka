#ifndef SPIKEDETANALYSIS_H
#define SPIKEDETANALYSIS_H

#include "../Alenka-Signal/include/AlenkaSignal/spikedet.h"

namespace AlenkaSignal {
class OpenCLContext;
}
class OpenDataFile;
class QProgressDialog;

class SpikedetAnalysis {
  double spikeDuration;
  AlenkaSignal::OpenCLContext *context;
  DETECTOR_SETTINGS settings = AlenkaSignal::Spikedet::defaultSettings();
  CDetectorOutput *output = nullptr;
  CDischarges *discharges = nullptr;

public:
  SpikedetAnalysis(AlenkaSignal::OpenCLContext *context) : context(context) {}
  ~SpikedetAnalysis() {
    delete output;
    delete discharges;
  }

  CDetectorOutput *getOutput() { return output; }
  CDischarges *getDischarges() { return discharges; }

  void setSettings(const DETECTOR_SETTINGS &s) { settings = s; }
  DETECTOR_SETTINGS getSettings() const { return settings; }

  void setSpikeDuration(double val) { spikeDuration = val; }

  void runAnalysis(OpenDataFile *file, QProgressDialog *progress,
                   bool originalSpikedet);

  static void analyseCommandLineFile();
};

#endif // SPIKEDETANALYSIS_H
