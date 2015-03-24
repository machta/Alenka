#include "testwindow.h"
#include "ui_testwindow.h"

#include <QFileDialog>
#include <QFileInfo>

TestWindow::TestWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::TestWindow)
{
	ui->setupUi(this);
}

TestWindow::~TestWindow()
{
	delete ui;

	delete file;
}

void TestWindow::on_actionEventTypeManager_triggered()
{
	if (file != nullptr)
	{
		EventTypeManager* manager = new EventTypeManager(file->getEventTypeTable(), this);
		manager->show();
	}
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
	}
}

void TestWindow::on_actionCloseFile_triggered()
{
	delete file;
	file = nullptr;
	ui->signalViewer->changeFile(nullptr);
}
