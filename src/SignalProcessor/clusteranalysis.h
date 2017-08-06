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

public:
  void runAnalysis(OpenDataFile *file, QWidget *parent) override;
  std::string name() override { return "Cluster Analysis"; }

  void setDischarges(CDischarges *ptr) { discharges = ptr; }

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
