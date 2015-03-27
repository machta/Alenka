#include "eventtypemanager.h"
#include "ui_eventtypemanager.h"

EventTypeManager::EventTypeManager(QWidget* parent) : QDialog(parent, Qt::Window), ui(new Ui::EventTypeManager)
{
	ui->setupUi(this);

	setWindowTitle("Event Type Manager");
}

EventTypeManager::~EventTypeManager()
{
	delete ui;
}

void EventTypeManager::setModel(EventTypeTable *model)
{
	ui->eventTypeTableView->setModel(model);
}
