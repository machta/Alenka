#include "montagemanager.h"

#include "../DataModel/opendatafile.h"

void MontageManager::insertRowBack()
{
	file->dataModel->montageTable()->insertRows(file->dataModel->montageTable()->rowCount());
}
