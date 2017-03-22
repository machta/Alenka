#include "eventmanager.h"

#include "../DataModel/opendatafile.h"
#include <AlenkaFile/datafile.h>
#include "../canvas.h"

#include <QTableView>
#include <QPushButton>
#include <QAction>

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace
{

AbstractEventTable* currentEventTable(DataModel* dataModel)
{
	return dataModel->montageTable()->eventTable(OpenDataFile::infoTable.getSelectedMontage());
}

} // namespace

EventManager::EventManager(QWidget* parent) : Manager(parent)
{
	QAction* goToAction = new QAction("Go To", this);
	goToAction->setShortcut(QKeySequence("g"));
	goToAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(goToAction, SIGNAL(triggered()), this, SLOT(goToEvent()));
	tableView->addAction(goToAction);

	QPushButton* goToButton = new QPushButton("Go To", this);
	connect(goToButton, SIGNAL(clicked()), this, SLOT(goToEvent()));
	addButton(goToButton);
}

void EventManager::insertRowBack()
{
	int rc = currentEventTable(file->getDataModel())->rowCount();
	currentEventTable(file->getDataModel())->insertRows(rc);
}

// TODO: Don't constrain goto to the dimensions of the slider, but rather make sure time line is always correctly positioned.
void EventManager::goToEvent()
{
	auto currentIndex = tableView->selectionModel()->currentIndex();

	if (file && currentIndex.isValid())
	{
		InfoTable& it = OpenDataFile::infoTable;

		double ratio = static_cast<double>(file->getSamplesRecorded())/it.getVirtualWidth();
		int col = static_cast<int>(Event::Index::position);
		auto index = tableView->model()->index(currentIndex.row(), col);

		double position = tableView->model()->data(index, Qt::EditRole).toInt();
		position /= ratio;
		position -= canvas->width()*it.getPositionIndicator();

		int intPosition = position;
		intPosition = min(max(0, intPosition), it.getVirtualWidth() - canvas->width() - 1);

		it.setPosition(intPosition);
	}
}
