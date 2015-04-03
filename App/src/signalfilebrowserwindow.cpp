#include "signalfilebrowserwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>

using namespace std;

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	//QLayout* windowLayout = layout();
	//layout->setContentsMargins(0, 0, 0, 0);
	//layout->setSpacing(0);

	// Construct child widgets.
	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

	montageManager = new MontageManager(this);
	eventManager = new EventManager(this);
	eventTypeManager = new EventTypeManager(this);

	// Construct actions.
	QAction* openFileAction = new QAction("&Open File", this);
	openFileAction->setShortcuts(QKeySequence::Open);
	openFileAction->setToolTip("Open an existing file.");
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	QAction* closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcuts(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	QAction* showMontageManagerAction = new QAction("Montage Manager", this);
	showMontageManagerAction->setToolTip("Show Montage Manager.");
	connect(showMontageManagerAction, SIGNAL(triggered()), this, SLOT(showMontageManager()));

	QAction* showEventManagerAction = new QAction("Event Manager", this);
	showEventManagerAction->setToolTip("Show Event Manager.");
	connect(showEventManagerAction, SIGNAL(triggered()), this, SLOT(showEventManager()));

	QAction* showEventTypeManagerAction = new QAction("Event Type Manager", this);
	showEventTypeManagerAction->setToolTip("Show Event Type Manager.");
	connect(showEventTypeManagerAction, SIGNAL(triggered()), this, SLOT(showEventTypeManager()));

	// Construct menus.
	QMenu* fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);

	QMenu* windowMenu = menuBar()->addMenu("&Window");
	windowMenu->addAction(showMontageManagerAction);
	windowMenu->addAction(showEventManagerAction);
	windowMenu->addAction(showEventTypeManagerAction);

	// Construct toolbars.

}

SignalFileBrowserWindow::~SignalFileBrowserWindow()
{
	delete file;
}

void SignalFileBrowserWindow::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "GDF file (*.gdf)");

	if (fileName.isNull() == false)
	{
		QFileInfo fi(fileName);

		delete file;
		file = new GDF2((fi.path() + QDir::separator() + fi.completeBaseName()).toStdString());

		signalViewer->changeFile(file);

		// Update the managers.
		montageManager->setModel(file->getMontageTables()->front());
		eventManager->setModel(file->getMontageTables()->front()->getEventTable());
		eventTypeManager->setModel(file->getEventTypeTable());
	}
}

void SignalFileBrowserWindow::closeFile()
{
	delete file;
	file = nullptr;
	signalViewer->changeFile(nullptr);
}

void SignalFileBrowserWindow::showMontageManager()
{
	montageManager->show();
}

void SignalFileBrowserWindow::showEventManager()
{
	eventManager->show();
}

void SignalFileBrowserWindow::showEventTypeManager()
{
	eventTypeManager->show();
}

