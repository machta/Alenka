#ifndef MODIFIEDSPIKEDETANALYSIS_H
#define MODIFIEDSPIKEDETANALYSIS_H

#include "spikedetanalysis.h"

namespace AlenkaSignal {
class OpenCLContext;
}
class OpenDataFile;
class QProgressDialog;

class ModifiedSpikedetAnalysis : public SpikedetAnalysis {
public:
  ModifiedSpikedetAnalysis(AlenkaSignal::OpenCLContext *context)
      : SpikedetAnalysis(context) {}

  std::string name() override { return "Optimized Spikedet Analysis"; }

protected:
  bool originalSpikedet() override { return false; }
};

#endif // MODIFIEDSPIKEDETANALYSIS_H
