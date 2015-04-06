#include "trackmanager.h"

#include "DataFile/tracktable.h"
#include "trackmanagerdelegate.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QHeaderView>

TrackManager::TrackManager(QWidget* parent) : QWidget(parent)
{
	tableView = new QTableView(this);
	tableView->setSortingEnabled(true);
	tableView->sortByColumn(0, Qt::AscendingOrder);
	tableView->setItemDelegate(new TrackManagerDelegate);

	QPushButton* addRowButton = new QPushButton("Add Row", this);
	connect(addRowButton, SIGNAL(clicked()), this, SLOT(addRow()));

	QPushButton* removeRowButton = new QPushButton("Remove Row", this);
	connect(removeRowButton, SIGNAL(clicked()), this, SLOT(removeRow()));

	QVBoxLayout* box1 = new QVBoxLayout;
	QHBoxLayout* box2 = new QHBoxLayout;
	box2->addWidget(addRowButton);
	box2->addWidget(removeRowButton);
	box1->addLayout(box2);
	box1->addWidget(tableView);
	setLayout(box1);
}

TrackManager::~TrackManager()
{
}

void TrackManager::setModel(TrackTable* model)
{
	tableView->setModel(model);

	tableView->sortByColumn(tableView->horizontalHeader()->sortIndicatorSection(), tableView->horizontalHeader()->sortIndicatorOrder());
}

void TrackManager::addRow()
{
	tableView->model()->insertRow(tableView->model()->rowCount());
}

void TrackManager::removeRow()
{
	QModelIndex currentIndex = tableView->selectionModel()->currentIndex();
	if (currentIndex.isValid())
	{
		tableView->model()->removeRow(currentIndex.row());
	}
}
