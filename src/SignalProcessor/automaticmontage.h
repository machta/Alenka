#ifndef AUTOMATICMONTAGE_H
#define AUTOMATICMONTAGE_H

#include <string>

namespace AlenkaFile {
class AbstractTrackTable;
}

class AutomaticMontage {
public:
  std::string getName() const { return name; }
  virtual void fillTrackTable(const AlenkaFile::AbstractTrackTable * /*source*/,
                              AlenkaFile::AbstractTrackTable * /*output*/) {}

protected:
  std::string name = "Empty Montage";
};

#endif // AUTOMATICMONTAGE_H
