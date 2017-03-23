#include "eventtypemanager.h"

#include "../DataModel/opendatafile.h"

void EventTypeManager::insertRowBack()
{
	file->dataModel->eventTypeTable()->insertRows(file->dataModel->eventTypeTable()->rowCount());
}
