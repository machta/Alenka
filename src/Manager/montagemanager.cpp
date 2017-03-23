#include "montagemanager.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

void MontageManager::insertRowBack()
{
	int rc = file->dataModel->montageTable()->rowCount();
	file->undoFactory->insertMontage(rc, 1, "add Montage row");
}
