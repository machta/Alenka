#ifndef SPIKEDETANALYSIS_H
#define SPIKEDETANALYSIS_H

#include "../../Alenka-Signal/include/AlenkaSignal/spikedet.h"
#include "analysis.h"

#include <memory>

class OpenDataFile;

class SpikedetAnalysis : public Analysis {
  double spikeDuration;
  DETECTOR_SETTINGS settings = AlenkaSignal::Spikedet::defaultSettings();
  std::unique_ptr<CDetectorOutput> output;
  std::unique_ptr<CDischarges> discharges;

public:
  void runAnalysis(OpenDataFile *file, QWidget *parent) override;
  std::string name() override { return "Spikedet Analysis"; }

  CDetectorOutput *getOutput() { return output.get(); }
  CDischarges *getDischarges() { return discharges.get(); }

  void setSettings(const DETECTOR_SETTINGS &s) { settings = s; }
  DETECTOR_SETTINGS getSettings() const { return settings; }

  void setSpikeDuration(double val) { spikeDuration = val; }

  static void analyseCommandLineFile();

protected:
  virtual bool originalSpikedet() { return true; }
};

#endif // SPIKEDETANALYSIS_H
