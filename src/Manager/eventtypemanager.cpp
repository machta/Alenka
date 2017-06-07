#include "eventtypemanager.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

bool EventTypeManager::insertRowBack() {
  if (file) {
    int rc = file->dataModel->eventTypeTable()->rowCount();
    file->undoFactory->insertEventType(rc, 1, "add EventType row");
    return true;
  }

  return false;
}
