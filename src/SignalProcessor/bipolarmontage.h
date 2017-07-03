#ifndef BIPOLARMONTAGE_H
#define BIPOLARMONTAGE_H

#include "automaticmontage.h"

#include <vector>

class BipolarMontage : public AutomaticMontage {
public:
  BipolarMontage() { name = "Bipolar Montage"; }

  void fillTrackTable(const AlenkaFile::AbstractTrackTable *source,
                      AlenkaFile::AbstractTrackTable *output) override;

protected:
  virtual int matchPair(int i, const std::vector<std::string> &prefixes,
                        const std::vector<int> &indexes);
};

class BipolarNeighboursMontage : public BipolarMontage {
public:
  BipolarNeighboursMontage() { name = "Bipolar Montage (neighbours only)"; }

protected:
  int matchPair(int i, const std::vector<std::string> &prefixes,
                const std::vector<int> &indexes) override;
};

#endif // BIPOLARMONTAGE_H
