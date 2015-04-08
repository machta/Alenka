#include "eventmanager.h"

#include "eventmanagerdelegate.h"

#include <QTableView>

EventManager::EventManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventManagerDelegate(tableView));
}

EventManager::~EventManager()
{
}

