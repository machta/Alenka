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
