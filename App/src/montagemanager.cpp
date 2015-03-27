#include "montagemanager.h"
#include "ui_montagemanager.h"

MontageManager::MontageManager(QWidget* parent) : QDialog(parent, Qt::Window), ui(new Ui::MontageManager)
{
	ui->setupUi(this);

	setWindowTitle("Montage Manager");
}

MontageManager::~MontageManager()
{
	delete ui;
}

void MontageManager::setModel(MontageTable *model)
{
	ui->montageTableView->setModel(model);
}
