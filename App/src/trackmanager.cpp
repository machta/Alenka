#include "trackmanager.h"

#include "trackmanagerdelegate.h"

#include <QTableView>

TrackManager::TrackManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new TrackManagerDelegate(tableView));
}

TrackManager::~TrackManager()
{
}
