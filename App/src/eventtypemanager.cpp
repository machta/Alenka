#include "eventtypemanager.h"
#include "ui_eventtypemanager.h"

EventTypeManager::EventTypeManager(EventTypeTable* model, QWidget* parent) : QDialog(parent), ui(new Ui::EventTypeManager)
{
	ui->setupUi(this);

	ui->eventTypeTableView->setModel(model);
}

EventTypeManager::~EventTypeManager()
{
	delete ui;
}
