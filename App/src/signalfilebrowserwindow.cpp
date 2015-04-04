#include "signalfilebrowserwindow.h"

#include "signalviewer.h"
#include "DataFile/gdf2.h"
#include "trackmanager.h"
#include "eventmanager.h"
#include "eventtypemanager.h"

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDockWidget>

using namespace std;

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

	// Construct dock widgets.
	//setDockNestingEnabled(true);
	//dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	QDockWidget* dockWidget1 = new QDockWidget("Track Manager", this);
	trackManager = new TrackManager(this);
	dockWidget1->setWidget(trackManager);

	QDockWidget* dockWidget2 = new QDockWidget("Event Manager", this);
	eventManager = new EventManager(this);
	dockWidget2->setWidget(eventManager);

	QDockWidget* dockWidget3 = new QDockWidget("EventType Manager", this);
	eventTypeManager = new EventTypeManager(this);
	dockWidget3->setWidget(eventTypeManager);

	addDockWidget(Qt::RightDockWidgetArea, dockWidget1);
	tabifyDockWidget(dockWidget1, dockWidget2);
	tabifyDockWidget(dockWidget2, dockWidget3);

	// Construct File actions.
	QAction* openFileAction = new QAction("&Open File", this);
	openFileAction->setShortcuts(QKeySequence::Open);
	openFileAction->setToolTip("Open an existing file.");
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	QAction* closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcuts(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	QAction* saveFileAction = new QAction("Save File", this);
	saveFileAction->setShortcuts(QKeySequence::Save);
	saveFileAction->setToolTip("Save the currently opened file.");
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	// Construct File menu.
	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);
	fileMenu->addAction(saveFileAction);

	// Toolbars.
	const int spacing = 3;

	// Construct File toolbar.
	QToolBar* fileToolBar = addToolBar("File");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

	// Construct Filter toolbar.
	QToolBar* filterToolBar = addToolBar("Filter");
	filterToolBar->layout()->setSpacing(spacing);

	filterToolBar->addWidget(new QLabel("LF:", this));
	lowpassComboBox = new QComboBox(this);
	lowpassComboBox->setEditable(true);
	filterToolBar->addWidget(lowpassComboBox);

	filterToolBar->addWidget(new QLabel("HF:", this));
	highpassComboBox = new QComboBox(this);
	highpassComboBox->setEditable(true);
	filterToolBar->addWidget(highpassComboBox);

	notchCheckBox = new QCheckBox("Notch:", this);
	notchCheckBox->setLayoutDirection(Qt::RightToLeft);
	filterToolBar->addWidget(notchCheckBox);

	// Construct Montage Toolbar.
	QToolBar* montageToolBar = addToolBar("Filter");
	montageToolBar->layout()->setSpacing(spacing);

	montageToolBar->addWidget(new QLabel("Montage:", this));
	montageComboBox = new QComboBox(this);
	montageToolBar->addWidget(montageComboBox);
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
		trackManager->setModel(file->getMontageTable()->getTrackTables()->front());
		eventManager->setModel(file->getMontageTable()->getEventTables()->front());
		eventTypeManager->setModel(file->getEventTypeTable());

		InfoTable* it = file->getInfoTable();

		// Update Filter toolbar.
		QStringList comboOptions;

		comboOptions << "---" << "0" << "5" << "10";

		for (int i = 50; i <= file->getSamplingFrequency()/2; i *= 2)
		{
			comboOptions << QString::number(i);
		}

		lowpassComboBox->clear();
		lowpassComboBox->addItems(comboOptions);
		connect(lowpassComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(lowpassComboBoxUpdate(QString)));
		connect(it, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(lowpassComboBoxUpdate(double)));
		emit it->lowpassFrequencyChanged(it->getLowpassFrequency());

		highpassComboBox->clear();
		highpassComboBox->addItems(comboOptions);
		connect(highpassComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(highpassComboBoxUpdate(QString)));
		connect(it, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(highpassComboBoxUpdate(double)));
		emit it->highpassFrequencyChanged(it->getHighFrequency());

		connect(notchCheckBox, SIGNAL(toggled(bool)), it, SLOT(setNotch(bool)));
		connect(it, SIGNAL(notchChanged(bool)), notchCheckBox, SLOT(setChecked(bool)));
		emit it->notchChanged(it->getNotch());
	}
}

void SignalFileBrowserWindow::closeFile()
{
	delete file;
	file = nullptr;
	signalViewer->changeFile(nullptr);
}

void SignalFileBrowserWindow::saveFile()
{
	if (file != nullptr)
	{
		file->save();
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(const QString& text)
{
	if (file != nullptr)
	{
		bool ok;
		double tmp = text.toDouble(&ok);
		file->getInfoTable()->setLowpassFrequency(ok ? tmp : 1000*1000*1000);
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(double value)
{
	if (file != nullptr)
	{
		lowpassComboBox->setCurrentText(QString::number(value, 'f'));
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(const QString& text)
{
	if (file != nullptr)
	{
		bool ok;
		double tmp = text.toDouble(&ok);
		file->getInfoTable()->setHighFrequency(ok ? tmp : -1000*1000*1000);
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(double value)
{
	if (file != nullptr)
	{
		highpassComboBox->setCurrentText(QString::number(value, 'f'));
	}
}
