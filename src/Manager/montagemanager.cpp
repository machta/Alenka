#include "montagemanager.h"

#include <AlenkaFile/datafile.h>

void MontageManager::insertRowBack()
{
	file->getDataModel().montageTable->insertRows(file->getDataModel().montageTable->rowCount());
}
