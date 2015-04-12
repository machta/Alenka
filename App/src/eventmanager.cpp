#include "eventmanager.h"

#include "DataFile/datafile.h"
#include "DataFile/eventtable.h"
#include "DataFile/infotable.h"
#include "eventmanagerdelegate.h"

#include <QTableView>
#include <QPushButton>
#include <QAction>

EventManager::EventManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventManagerDelegate(tableView));

	QAction* goToAction = new QAction("Go To", this);
	goToAction->setShortcut(QKeySequence("g"));
	goToAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(goToAction, SIGNAL(triggered()), this, SLOT(goToEvent()));
	tableView->addAction(goToAction);

	QPushButton* goToButton = new QPushButton("Go To (G)", this);
	connect(goToButton, SIGNAL(clicked()), this, SLOT(goToEvent()));
	addButton(goToButton);
}

EventManager::~EventManager()
{
}

void EventManager::goToEvent()
{
	auto index = tableView->selectionModel()->currentIndex();

	if (index.isValid() && file != nullptr)
	{
		int position = tableView->model()->data(tableView->model()->index(index.row(), static_cast<int>(EventTable::Column::position))).toInt();

		double ratio = static_cast<double>(file->getSamplesRecorded())/file->getInfoTable()->getVirtualWidth();

		file->getInfoTable()->setPosition(position/ratio);
	}
}
