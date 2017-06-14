#include "trackmanager.h"

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

using namespace AlenkaFile;

bool TrackManager::insertRowBack() {
  int selected = OpenDataFile::infoTable.getSelectedMontage();

  if (file && selected != 0) {
    auto montageTable = file->dataModel->montageTable();
    assert(0 < montageTable->rowCount());

    int rc = montageTable->trackTable(selected)->rowCount();
    file->undoFactory->insertTrack(selected, rc, 1, "add Track row");

    return true;
  }

  return false;
}
