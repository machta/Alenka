#include "trackmanager.h"

#include "DataFile/tracktable.h"
#include "trackmanagerdelegate.h"

#include <QTableView>
#include <QPushButton>

TrackManager::TrackManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new TrackManagerDelegate(tableView));
}

TrackManager::~TrackManager()
{
}
