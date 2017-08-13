#ifndef CLUSTERANALYSIS_H
#define CLUSTERANALYSIS_H

#include "../../Alenka-Signal/include/AlenkaSignal/cluster.h"
#include "analysis.h"
#include "spikedetanalysis.h"

#include <memory>

namespace AlenkaSignal {
class OpenCLContext;
}
class OpenDataFile;

class ClusterAnalysis : public Analysis {
  std::unique_ptr<AlenkaSignal::Cluster> cluster;
  CDischarges *discharges = nullptr;
  double spikeDuration = 1;

public:
  void runAnalysis(OpenDataFile *file, QWidget *parent) override;
  std::string name() override { return "Cluster Analysis"; }

  void setDischarges(CDischarges *ptr) { discharges = ptr; }
  void setSpikeDuration(double duration) { spikeDuration = duration; }

protected:
  virtual bool pcaCentering() { return false; }
};

class CenteringClusterAnalysis : public ClusterAnalysis {
public:
  std::string name() override { return "Centering PCA Cluster Analysis"; }

protected:
  virtual bool pcaCentering() { return true; }
};

#endif // CLUSTERANALYSIS_H
