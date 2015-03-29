#include "eventmanager.h"
#include "ui_eventmanager.h"

EventManager::EventManager(QWidget* parent) : QDialog(parent, Qt::Window), ui(new Ui::EventManager)
{
	ui->setupUi(this);

	setWindowTitle("Event Manager");
}

EventManager::~EventManager()
{
	delete ui;
}

void EventManager::setModel(EventTable *model)
{
	ui->eventTableView->setModel(model);
}

void EventManager::on_addRowButton_clicked()
{
	ui->eventTableView->model()->insertRow(ui->eventTableView->model()->rowCount());
}

void EventManager::on_removeRowButton_clicked()
{
	QModelIndex currentIndex = ui->eventTableView->selectionModel()->currentIndex();
	if (currentIndex.isValid())
	{
		ui->eventTableView->model()->removeRow(currentIndex.row());
	}
}
