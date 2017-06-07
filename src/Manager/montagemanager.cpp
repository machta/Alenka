#include "montagemanager.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

bool MontageManager::insertRowBack() {
  if (file) {
    int rc = file->dataModel->montageTable()->rowCount();
    file->undoFactory->insertMontage(rc, 1, "add Montage row");
    return true;
  }

  return false;
}
