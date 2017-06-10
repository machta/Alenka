#ifndef SPIKEDETANALYSIS_H
#define SPIKEDETANALYSIS_H

#include "../Alenka-Signal/include/AlenkaSignal/spikedet.h"

#include <memory>

namespace AlenkaSignal {
class OpenCLContext;
}
class OpenDataFile;
class QProgressDialog;

class SpikedetAnalysis {
  double spikeDuration;
  AlenkaSignal::OpenCLContext *context;
  DETECTOR_SETTINGS settings = AlenkaSignal::Spikedet::defaultSettings();
  std::unique_ptr<CDetectorOutput> output;
  std::unique_ptr<CDischarges> discharges;

public:
  SpikedetAnalysis(AlenkaSignal::OpenCLContext *context) : context(context) {}

  CDetectorOutput *getOutput() { return output.get(); }
  CDischarges *getDischarges() { return discharges.get(); }

  void setSettings(const DETECTOR_SETTINGS &s) { settings = s; }
  DETECTOR_SETTINGS getSettings() const { return settings; }

  void setSpikeDuration(double val) { spikeDuration = val; }

  void runAnalysis(OpenDataFile *file, QProgressDialog *progress,
                   bool originalSpikedet);

  static void analyseCommandLineFile();
};

#endif // SPIKEDETANALYSIS_H
