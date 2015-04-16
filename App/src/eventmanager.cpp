#include "eventmanager.h"

#include "DataFile/datafile.h"
#include "DataFile/eventtable.h"
#include "DataFile/infotable.h"
#include "eventmanagerdelegate.h"
#include "canvas.h"

#include <QTableView>
#include <QPushButton>
#include <QAction>

#include <algorithm>

using namespace std;

EventManager::EventManager(QWidget* parent) : Manager(parent)
{
	tableView->setItemDelegate(new EventManagerDelegate(tableView));

	QAction* goToAction = new QAction("Go To", this);
	goToAction->setShortcut(QKeySequence("g"));
	goToAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(goToAction, SIGNAL(triggered()), this, SLOT(goToEvent()));
	tableView->addAction(goToAction);

	QPushButton* goToButton = new QPushButton("Go To", this);
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
		double ratio = static_cast<double>(file->getSamplesRecorded())/file->getInfoTable()->getVirtualWidth();

		double position = tableView->model()->data(tableView->model()->index(index.row(), static_cast<int>(EventTable::Column::position)), Qt::EditRole).toInt();
		position /= ratio;
		position -= canvas->width()*file->getInfoTable()->getPositionIndicator();

		int intPosition = position;
		intPosition = min(max(0, intPosition), file->getInfoTable()->getVirtualWidth() - canvas->width() - 1);

		file->getInfoTable()->setPosition(intPosition);
	}
}
