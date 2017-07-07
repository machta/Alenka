#ifndef AUTOMATICMONTAGE_H
#define AUTOMATICMONTAGE_H

#include "../DataModel/undocommandfactory.h"

#include <string>

namespace AlenkaFile {
class AbstractTrackTable;
}

class AutomaticMontage {
public:
  std::string getName() const { return name; }
  virtual void fillTrackTable(const AlenkaFile::AbstractTrackTable * /*source*/,
                              const AlenkaFile::AbstractTrackTable * /*output*/,
                              int /*outputIndex*/,
                              UndoCommandFactory * /*undoFactory*/) {}

protected:
  std::string name = "Empty Montage";
};

#endif // AUTOMATICMONTAGE_H
