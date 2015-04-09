#include "eventtypemanager.h"

#include "DataFile/eventtypetable.h"
#include "eventtypemanagerdelegate.h"

#include <QTableView>
#include <QPushButton>

EventTypeManager::EventTypeManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventTypeManagerDelegate(tableView));
}

EventTypeManager::~EventTypeManager()
{
}
