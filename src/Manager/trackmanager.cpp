#include "trackmanager.h"

#include <AlenkaFile/datafile.h>
#include "../signalfilebrowserwindow.h"

using namespace AlenkaFile;

namespace
{

AbstractTrackTable* currentTrackTable(DataModel dataModel)
{
	return dataModel.montageTable->trackTable(SignalFileBrowserWindow::infoTable.getSelectedMontage());
}

} // namespace

void TrackManager::insertRowBack()
{
	int rc = currentTrackTable(file->getDataModel())->rowCount();
	currentTrackTable(file->getDataModel())->insertRows(rc);
}
