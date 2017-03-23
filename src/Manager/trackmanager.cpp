#include "trackmanager.h"

#include <AlenkaFile/datafile.h>
#include "../DataModel/opendatafile.h"

using namespace AlenkaFile;

namespace
{

AbstractTrackTable* currentTrackTable(DataModel* dataModel)
{
	return dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
}

} // namespace

void TrackManager::insertRowBack()
{
	int rc = currentTrackTable(file->dataModel)->rowCount();
	currentTrackTable(file->dataModel)->insertRows(rc);
}
