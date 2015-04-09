#include "eventmanager.h"

#include "DataFile/eventtable.h"
#include "eventmanagerdelegate.h"

#include <QTableView>
#include <QPushButton>

EventManager::EventManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventManagerDelegate(tableView));
}

EventManager::~EventManager()
{
}
