#ifndef DEFAULTMONTAGE_H
#define DEFAULTMONTAGE_H

#include "automaticmontage.h"

class DefaultMontage : public AutomaticMontage {
public:
  DefaultMontage() { name = "Default Montage"; }

  void fillTrackTable(const AlenkaFile::AbstractTrackTable *recordingMontage,
                      const AlenkaFile::AbstractTrackTable *output,
                      int outputIndex,
                      UndoCommandFactory *undoFactory) override;
};

#endif // DEFAULTMONTAGE_H
