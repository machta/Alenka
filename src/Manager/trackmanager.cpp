#include "trackmanager.h"

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

using namespace AlenkaFile;

bool TrackManager::insertRowBack() {
  if (file && 0 < file->dataModel->montageTable()->rowCount()) {
    int rc = file->dataModel->montageTable()
                 ->trackTable(OpenDataFile::infoTable.getSelectedMontage())
                 ->rowCount();
    file->undoFactory->insertTrack(OpenDataFile::infoTable.getSelectedMontage(),
                                   rc, 1, "add Track row");
    return true;
  }

  return false;
}
