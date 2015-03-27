#include "testwindow.h"
#include "ui_testwindow.h"

#include <QFileDialog>
#include <QFileInfo>

TestWindow::TestWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::TestWindow)
{
	ui->setupUi(this);

	montageManager = new MontageManager(this);
	eventManager = new EventManager(this);
	eventTypeManager = new EventTypeManager(this);
}

TestWindow::~TestWindow()
{
	delete ui;

	delete file;
}

void TestWindow::on_actionEventTypeManager_triggered()
{
	eventTypeManager->show();
}

void TestWindow::on_actionOpenFile_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "GDF file (*.gdf)");

	if (fileName.isNull() == false)
	{
		QFileInfo fi(fileName);

		delete file;
		file = new GDF2((fi.path() + QDir::separator() + fi.completeBaseName()).toStdString());

		ui->signalViewer->changeFile(file);

		// Update the managers.
		montageManager->setModel(file->getMontageTables()->front());
		eventManager->setModel(file->getMontageTables()->front()->getEventTable());
		eventTypeManager->setModel(file->getEventTypeTable());
	}
}

void TestWindow::on_actionCloseFile_triggered()
{
	delete file;
	file = nullptr;
	ui->signalViewer->changeFile(nullptr);
}

void TestWindow::on_actionEventManager_triggered()
{
	eventManager->show();
}

void TestWindow::on_actionMontageManager_triggered()
{
	montageManager->show();
}
