#include "eventtypemanager.h"

#include <AlenkaFile/datafile.h>

void EventTypeManager::insertRowBack()
{
	file->getDataModel().eventTypeTable->insertRows(file->getDataModel().eventTypeTable->rowCount());
}
