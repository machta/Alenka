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

using namespace std;

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	// Construct child widgets.
	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

	trackManager = new TrackManager(this);
	eventManager = new EventManager(this);
	eventTypeManager = new EventTypeManager(this);

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

	// Construct Window actions.
	QAction* showTrackManagerAction = new QAction("Track Manager", this);
	showTrackManagerAction->setCheckable(true);
	showTrackManagerAction->setToolTip("Show Track Manager window.");
	connect(showTrackManagerAction, SIGNAL(triggered(bool)), this, SLOT(showHideTrackManager(bool)));

	QAction* showEventManagerAction = new QAction("Event Manager", this);
	showEventManagerAction->setCheckable(true);
	showEventManagerAction->setToolTip("Show Event Manager window.");
	connect(showEventManagerAction, SIGNAL(triggered(bool)), this, SLOT(showHideEventManager(bool)));

	QAction* showEventTypeManagerAction = new QAction("Event Type Manager", this);
	showEventTypeManagerAction->setCheckable(true);
	showEventTypeManagerAction->setToolTip("Show Event Type Manager window.");
	connect(showEventTypeManagerAction, SIGNAL(triggered(bool)), this, SLOT(showHideEventTypeManager(bool)));

	// Construct File menu.
	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);
	fileMenu->addAction(saveFileAction);

	// Construct Window menu.
	QMenu* windowMenu = menuBar()->addMenu("&Window");

	windowMenu->addAction(showTrackManagerAction);
	windowMenu->addAction(showEventManagerAction);
	windowMenu->addAction(showEventTypeManagerAction);

	// Toolbars.
	const int spacing = 3;

	// Construct File toolbar.
	QToolBar* fileToolBar = addToolBar("File");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

	// Construct Window toolbar.
	QToolBar* windowToolBar = addToolBar("Window");
	windowToolBar->layout()->setSpacing(spacing);

	windowToolBar->addAction(showTrackManagerAction);
	windowToolBar->addAction(showEventManagerAction);
	windowToolBar->addAction(showEventTypeManagerAction);

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

		//connect(, SIGNAL(), , SLOT());

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

void SignalFileBrowserWindow::showHideTrackManager(bool checked)
{
	if (checked)
	{
		trackManager->show();
	}
	else
	{
		trackManager->hide();
	}
}

void SignalFileBrowserWindow::showHideEventManager(bool checked)
{
	if (checked)
	{
		eventManager->show();
	}
	else
	{
		eventManager->hide();
	}
}

void SignalFileBrowserWindow::showHideEventTypeManager(bool checked)
{
	if (checked)
	{
		eventTypeManager->show();
	}
	else
	{
		eventTypeManager->hide();
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

