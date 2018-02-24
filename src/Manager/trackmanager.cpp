#include "trackmanager.h"

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

using namespace AlenkaFile;

bool TrackManager::insertRowBack() {
  const int selected = OpenDataFile::infoTable.getSelectedMontage();

  // Montage with index 0 shouldn't be edited.
  if (file && 0 != selected) {
    auto montageTable = file->dataModel->montageTable();
    assert(0 < montageTable->rowCount());

    const int rc = montageTable->trackTable(selected)->rowCount();
    file->undoFactory->insertTrack(selected, rc, 1, "add Track row");

    return true;
  }

  return false;
}
