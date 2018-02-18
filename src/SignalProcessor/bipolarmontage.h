#ifndef BIPOLARMONTAGE_H
#define BIPOLARMONTAGE_H

#include "automaticmontage.h"

#include <vector>

/**
 * @brief Creates an bipolar montage.
 *
 * It takes the labels that start with a common prefix. Then it tries to match
 * pairs using the numeric suffix. For example labels
 * * T1
 * * T2
 * * T3
 * * T4
 * * A1
 * * A4
 *
 * will result in a new montage with tracks
 * * T1-2
 * * T3-4
 * * A1-4
 */
class BipolarMontage : public AutomaticMontage {
public:
  BipolarMontage() { name = "Bipolar Montage"; }

  void fillTrackTable(const AlenkaFile::AbstractTrackTable *recordingMontage,
                      const AlenkaFile::AbstractTrackTable *output,
                      int outputIndex,
                      UndoCommandFactory *undoFactory) override;

protected:
  virtual int matchPair(int i, const std::vector<std::string> &prefixes,
                        const std::vector<int> &indexes);
};

/**
 * @brief Creates an bipolar montage where the difference between indexes is at
 * most 1.
 *
 * It takes the labels that start with a common prefix. Then it tries to match
 * pairs using the numeric suffix. For example labels
 * * T1
 * * T2
 * * T3
 * * T4
 * * A1
 * * A4
 *
 * will result in a new montage with tracks:
 * * T1-2
 * * T3-4
 *
 * Note that the unlike with BipolarMontage the A1 and A4 will not be matched.
 */
class BipolarNeighboursMontage : public BipolarMontage {
public:
  BipolarNeighboursMontage() { name = "Bipolar Montage (neighbours only)"; }

protected:
  int matchPair(int i, const std::vector<std::string> &prefixes,
                const std::vector<int> &indexes) override;
};

#endif // BIPOLARMONTAGE_H
