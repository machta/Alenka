#include "trackmanager.h"

#include <AlenkaFile/datafile.h>
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

using namespace AlenkaFile;

void TrackManager::insertRowBack()
{
	int rc = file->dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage())->rowCount();
	file->undoFactory->insertTrack(OpenDataFile::infoTable.getSelectedMontage(), rc, 1, "add Track row");
}
