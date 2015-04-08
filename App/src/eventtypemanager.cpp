#include "eventtypemanager.h"

#include "eventtypemanagerdelegate.h"

#include <QTableView>

EventTypeManager::EventTypeManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventTypeManagerDelegate(tableView));
}

EventTypeManager::~EventTypeManager()
{
}
