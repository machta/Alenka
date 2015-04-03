#include "trackmanager.h"
#include "ui_trackmanager.h"

TrackManager::TrackManager(QWidget* parent) : QDialog(parent), ui(new Ui::TrackManager)
{
	ui->setupUi(this);

	setWindowTitle("Track Manager");
}

TrackManager::~TrackManager()
{
	delete ui;
}

void TrackManager::setModel(TrackTable* model)
{
	ui->trackTableView->setModel(model);
}
