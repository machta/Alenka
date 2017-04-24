#include "signalfilebrowserwindow.h"

#include "options.h"
#include "error.h"
#include "signalviewer.h"
#include "spikedetanalysis.h"
#include "myapplication.h"
#include "spikedetsettingsdialog.h"
#include "canvas.h"
#include "DataModel/undocommandfactory.h"
#include "DataModel/opendatafile.h"
#include "DataModel/vitnessdatamodel.h"
#include "Manager/trackmanager.h"
#include "Manager/eventmanager.h"
#include "Manager/eventtypemanager.h"
#include "Manager/montagemanager.h"
#include "Manager/filtermanager.h"
#include "Manager/eventtypetablemodel.h"
#include "Manager/montagetablemodel.h"
#include "Manager/eventtablemodel.h"
#include "Manager/tracktablemodel.h"
#include "SignalProcessor/signalprocessor.h"
#include "Sync/syncserver.h"
#include "Sync/syncclient.h"
#include "Sync/syncdialog.h"
#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaFile/gdf2.h>
#include <AlenkaFile/edf.h>
#include <AlenkaFile/mat.h>

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
#include <QStatusBar>
#include <QLayout>
#include <QInputDialog>
#include <QLocale>
#include <QProgressDialog>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QUndoStack>
#include <QCloseEvent>

#include <locale>
#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace
{

const char* TITLE = "Signal File Browser";

double byteArray2Double(const char* data)
{
	return *reinterpret_cast<const double*>(data);
}

void unpackMessage(const QByteArray& message, double* timePossition)
{
	assert(message.size() == (int)sizeof(double));
	*timePossition = byteArray2Double(message.data());
}

QByteArray packMessage(double timePossition)
{
	return QByteArray(reinterpret_cast<char*>(&timePossition), sizeof(double));
}

void executeWithCLocale(function<void ()> code)
{
	std::locale localeCopy;
	std::locale::global(std::locale("C"));

	code();

	std::locale::global(localeCopy);
}

} // namespace

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(TITLE);

	if (0 < PROGRAM_OPTIONS["kernelCacheSize"].as<int>())
		kernelCache = new KernelCache();
	else
		KernelCache::deleteCacheFile();

	autoSaveTimer = new QTimer(this);

	undoStack = new QUndoStack(this);
	connect(undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(cleanChanged(bool)));

	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

	openDataFile = new OpenDataFile();

	// Construct dock widgets.
	setDockNestingEnabled(true);

	QDockWidget* trackManagerDockWidget = new QDockWidget("Track Manager", this);
	trackManagerDockWidget->setObjectName("Track Manager QDockWidget");
	trackManager = new TrackManager(this);
	trackManagerDockWidget->setWidget(trackManager);

	QDockWidget* eventManagerDockWidget = new QDockWidget("Event Manager", this);
	eventManagerDockWidget->setObjectName("Event Manager QDockWidget");
	eventManager = new EventManager(this);
	eventManager->setReferences(signalViewer->getCanvas());
	eventManagerDockWidget->setWidget(eventManager);

	QDockWidget* eventTypeManagerDockWidget = new QDockWidget("EventType Manager", this);
	eventTypeManagerDockWidget->setObjectName("EventType Manager QDockWidget");
	eventTypeManager = new EventTypeManager(this);
	eventTypeManagerDockWidget->setWidget(eventTypeManager);

	QDockWidget* montageManagerDockWidget = new QDockWidget("Montage Manager", this);
	montageManagerDockWidget->setObjectName("Montage Manager QDockWidget");
	montageManager = new MontageManager(this);
	montageManagerDockWidget->setWidget(montageManager);

	QDockWidget* filterManagerDockWidget = new QDockWidget("Filter Manager", this);
	filterManagerDockWidget->setObjectName("Filter Manager QDockWidget");
	filterManager = new FilterManager(this);
	filterManagerDockWidget->setWidget(filterManager);

	addDockWidget(Qt::RightDockWidgetArea, trackManagerDockWidget);
	tabifyDockWidget(trackManagerDockWidget, eventManagerDockWidget);
	tabifyDockWidget(eventManagerDockWidget, eventTypeManagerDockWidget);
	tabifyDockWidget(eventTypeManagerDockWidget, montageManagerDockWidget);
	tabifyDockWidget(montageManagerDockWidget, filterManagerDockWidget);

	// Construct File actions.
	QAction* openFileAction = new QAction("&Open File", this);
	openFileAction->setShortcut(QKeySequence::Open);
	openFileAction->setToolTip("Open an existing file.");
	openFileAction->setStatusTip(openFileAction->toolTip());
	openFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcut(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	closeFileAction->setStatusTip(closeFileAction->toolTip());
	closeFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	saveFileAction = new QAction("Save File", this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveFileAction->setToolTip("Save the currently opened file.");
	saveFileAction->setStatusTip(saveFileAction->toolTip());
	saveFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	saveFileAction->setEnabled(false);
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	exportToEdfAction = new QAction("Export to EDF", this);
	//exportToEdfAction->setShortcut(QKeySequence::Open);
	exportToEdfAction->setToolTip("Export opened file to EDF.");
	exportToEdfAction->setStatusTip(exportToEdfAction->toolTip());
	//exportToEdfAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	connect(exportToEdfAction, SIGNAL(triggered()), this, SLOT(exportToEdf()));

	QAction* undoAction = undoStack->createUndoAction(this);
	undoAction->setShortcut(QKeySequence::Undo);
	//undoAction->setIcon(QIcon::fromTheme("edit-undo"));
	//undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));

	QAction* redoAction = undoStack->createRedoAction(this);
	redoAction->setShortcut(QKeySequence::Redo);
	//redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
	// TODO: Get proper icons for undo/redo.

	// Construct Zoom actions.
	QAction* horizontalZoomInAction = new QAction("Horizontal Zoom In", this);
	horizontalZoomInAction->setShortcut(QKeySequence("Alt++"));
	horizontalZoomInAction->setToolTip("Zoom in time line.");
	horizontalZoomInAction->setStatusTip(horizontalZoomInAction->toolTip());
	connect(horizontalZoomInAction, &QAction::triggered, [this] () {
		signalViewer->getCanvas()->horizontalZoom(false);
	});

	QAction* horizontalZoomOutAction = new QAction("Horizontal Zoom Out", this);
	horizontalZoomOutAction->setShortcut(QKeySequence("Alt+-"));
	horizontalZoomOutAction->setToolTip("Zoom out time line.");
	horizontalZoomOutAction->setStatusTip(horizontalZoomOutAction->toolTip());
	connect(horizontalZoomOutAction, &QAction::triggered, [this] () {
		signalViewer->getCanvas()->horizontalZoom(true);
	});

	QAction* verticalZoomInAction = new QAction("Vertical Zoom In", this);
	verticalZoomInAction->setShortcut(QKeySequence("Shift++"));
	verticalZoomInAction->setToolTip("Zoom in amplitudes of signals.");
	verticalZoomInAction->setStatusTip(verticalZoomInAction->toolTip());
	connect(verticalZoomInAction, &QAction::triggered, [this] () {
		signalViewer->getCanvas()->verticalZoom(false);
	});

	QAction* verticalZoomOutAction = new QAction("Vertical Zoom Out", this);
	verticalZoomOutAction->setShortcut(QKeySequence("Shift+-"));
	verticalZoomOutAction->setToolTip("Zoom out amplitudes of signals.");
	verticalZoomOutAction->setStatusTip(verticalZoomOutAction->toolTip());
	connect(verticalZoomOutAction, &QAction::triggered, [this] () {
		signalViewer->getCanvas()->verticalZoom(true);
	});

	// Construct Spikedet actions.
	runSpikedetAction = new QAction(QIcon(":/icons/play.png"), "Run Spikedet Analysis", this);
	runSpikedetAction->setToolTip("Run Spikedet analysis on the current montage.");
	runSpikedetAction->setStatusTip(runSpikedetAction->toolTip());
	connect(runSpikedetAction, &QAction::triggered, [this] () { runSpikedet(); } );

	QAction* spikedetSettingsAction = new QAction(QIcon(":/icons/settings.png"), "Spikedet Settings", this);
	spikedetSettingsAction->setToolTip("Change Spikedet settings.");
	spikedetSettingsAction->setStatusTip(spikedetSettingsAction->toolTip());
	connect(spikedetSettingsAction, &QAction::triggered, [this] () {
		AlenkaSignal::DETECTOR_SETTINGS settings = spikedetAnalysis->getSettings();
		double newDuration = spikeDuration;
		bool newDecimation = originalDecimation;

		SpikedetSettingsDialog dialog(&settings, &newDuration, &newDecimation, this);

		if (dialog.exec() == QDialog::Accepted)
		{
			spikedetAnalysis->setSettings(settings);
			spikeDuration = newDuration;
			originalDecimation = newDecimation;
		}
	});

	// Construct Time Mode action group.
	timeModeActionGroup = new QActionGroup(this);

	QAction* timeModeAction0 = new QAction("Sample", this);
	timeModeAction0->setToolTip("Samples from the start.");
	timeModeAction0->setStatusTip(timeModeAction0->toolTip());
	timeModeAction0->setActionGroup(timeModeActionGroup);
	timeModeAction0->setCheckable(true);
	connect(timeModeAction0, &QAction::triggered, [this] () { mode(0); });

	QAction* timeModeAction1 = new QAction("Offset", this);
	timeModeAction1->setToolTip("Time offset from the start.");
	timeModeAction1->setStatusTip(timeModeAction1->toolTip());
	timeModeAction1->setActionGroup(timeModeActionGroup);
	timeModeAction1->setCheckable(true);
	connect(timeModeAction1, &QAction::triggered, [this] () { mode(1); });

	QAction* timeModeAction2 = new QAction("Real", this);
	timeModeAction2->setToolTip("Real time.");
	timeModeAction2->setStatusTip(timeModeAction2->toolTip());
	timeModeAction2->setActionGroup(timeModeActionGroup);
	timeModeAction2->setCheckable(true);
	connect(timeModeAction2, &QAction::triggered, [this] () { mode(2); });

	// Construct Time Line Interval action group.
	timeLineIntervalActionGroup = new QActionGroup(this);

	QAction* timeLineOffAction = new QAction("Off", this);
	timeLineOffAction->setToolTip("Turn off the time lines.");
	timeLineOffAction->setStatusTip(timeLineOffAction->toolTip());
	connect(timeLineOffAction, &QAction::triggered, [this] () {
		if (file)
			OpenDataFile::infoTable.setTimeLineInterval(0);
	});

	setTimeLineIntervalAction = new QAction("Set", this);
	setTimeLineIntervalAction->setActionGroup(timeLineIntervalActionGroup);
	connect(setTimeLineIntervalAction, &QAction::triggered, [this] () {
		if (file)
		{
			double value = OpenDataFile::infoTable.getTimeLineInterval();
			if (value == 0)
				value = 1;

			bool ok;
			value = QInputDialog::getDouble(this, "Set the interval", "Please, enter the value for the time line interval here:", value, 0, 1000*1000*1000, 2, &ok);

			if (ok)
				OpenDataFile::infoTable.setTimeLineInterval(value);
		}
	});

	// Construct SyncDialog.
	syncServer = new SyncServer();
	syncClient = new SyncClient();
	syncDialog = new SyncDialog(syncServer, syncClient, this);

	QAction* showSyncDialog = new QAction("Show Sync Dialog", this);
	connect(showSyncDialog, SIGNAL(triggered(bool)), syncDialog, SLOT(show()));

	synchronize = new QAction("Synchronize", this);
	synchronize->setCheckable(true);
	synchronize->setChecked(true);

	connect(syncServer, SIGNAL(messageReceived(QByteArray)), this, SLOT(receiveSyncMessage(QByteArray)));
	connect(syncClient, SIGNAL(messageReceived(QByteArray)), this, SLOT(receiveSyncMessage(QByteArray)));

	// Tool bars.
	const int spacing = 3;

	// Construct File tool bar.
	QToolBar* fileToolBar = addToolBar("File Tool Bar");
	fileToolBar->setObjectName("File QToolBar");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);
	addAction(undoAction); // Add these to this widget to keep the shortcuts working, but do't include it in the toolbar.
	addAction(redoAction);

	// Construct Filter tool bar.
	QToolBar* filterToolBar = addToolBar("Filter Tool Bar");
	filterToolBar->setObjectName("Filter QToolBar");
	filterToolBar->layout()->setSpacing(spacing);

	QLabel* label = new QLabel("LF:", this);
	label->setToolTip("Low-pass Filter frequency");
	filterToolBar->addWidget(label);
	lowpassComboBox = new QComboBox(this);
	lowpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	lowpassComboBox->setMaximumWidth(150);
	lowpassComboBox->setEditable(true);
	filterToolBar->addWidget(lowpassComboBox);

	label = new QLabel("HF:", this);
	label->setToolTip("High-pass Filter frequency");
	filterToolBar->addWidget(label);
	highpassComboBox = new QComboBox(this);
	highpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	highpassComboBox->setMaximumWidth(150);
	highpassComboBox->setEditable(true);
	filterToolBar->addWidget(highpassComboBox);

	notchCheckBox = new QCheckBox("Notch:", this);
	notchCheckBox->setToolTip("Notch Filter on/off");
	notchCheckBox->setLayoutDirection(Qt::RightToLeft);
	filterToolBar->addWidget(notchCheckBox);

	// Construct Select tool bar.
	QToolBar* selectToolBar = addToolBar("Select Tool bar");
	selectToolBar->setObjectName("Select QToolBar");
	selectToolBar->layout()->setSpacing(spacing);

	label = new QLabel("Mont:", this);
	label->setToolTip("Montage");
	selectToolBar->addWidget(label);
	montageComboBox = new QComboBox(this);
	montageComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	montageComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(montageComboBox);

	label = new QLabel("ET:", this);
	label->setToolTip("Event Type");
	selectToolBar->addWidget(label);
	eventTypeComboBox = new QComboBox(this);
	eventTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	eventTypeComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(eventTypeComboBox);

	label = new QLabel("Res:", this);
	label->setToolTip("Vertical resolution -- units per cm");
	selectToolBar->addWidget(label);
	resolutionComboBox = new QComboBox(this);
	resolutionComboBox->setEditable(true);
	selectToolBar->addWidget(resolutionComboBox);

	label = new QLabel("Units:", this);
	label->setToolTip("Signal sample units");
	selectToolBar->addWidget(label);
	QComboBox* unitsComboBox = new QComboBox(this);
	unitsComboBox->addItems(QStringList() << QChar(0x00B5) + QString("V") << "mV" << "V" << "kV");
	unitsComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	selectToolBar->addWidget(unitsComboBox);
	connect(unitsComboBox, SIGNAL(currentIndexChanged(int)), &OpenDataFile::infoTable, SLOT(setSampleUnits(int)));
	connect(&OpenDataFile::infoTable, SIGNAL(sampleUnitsChanged(int)), unitsComboBox, SLOT(setCurrentIndex(int)));

	// Construct Zoom tool bar.
	QToolBar* zoomToolBar = addToolBar("Zoom Tool Bar");
	zoomToolBar->setObjectName("Zoom QToolBar");
	zoomToolBar->layout()->setSpacing(spacing);

	zoomToolBar->addAction(horizontalZoomInAction);
	zoomToolBar->addAction(horizontalZoomOutAction);
	zoomToolBar->addAction(verticalZoomInAction);
	zoomToolBar->addAction(verticalZoomOutAction);

	// Construct Spikedet tool bar.
	QToolBar* spikedetToolBar = addToolBar("Spikedet Tool Bar");
	spikedetToolBar->setObjectName("Spikedet QToolBar");
	spikedetToolBar->layout()->setSpacing(spacing);

	spikedetToolBar->addAction(runSpikedetAction);
	spikedetToolBar->addAction(spikedetSettingsAction);

	// Construct File menu.
	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);
	fileMenu->addAction(saveFileAction);
	fileMenu->addAction(exportToEdfAction);
	fileMenu->addSeparator();
	fileMenu->addAction(undoAction);
	fileMenu->addAction(redoAction);

	// Construct View menu.
	QMenu* viewMenu = menuBar()->addMenu("&View");

	viewMenu->addAction(horizontalZoomInAction);
	viewMenu->addAction(horizontalZoomOutAction);
	viewMenu->addAction(verticalZoomInAction);
	viewMenu->addAction(verticalZoomOutAction);
	viewMenu->addSeparator();

	QMenu* timeModeMenu = new QMenu("Time Mode", this);
	timeModeMenu->addAction(timeModeAction0);
	timeModeMenu->addAction(timeModeAction1);
	timeModeMenu->addAction(timeModeAction2);
	viewMenu->addMenu(timeModeMenu);

	QMenu* timeLineIntervalMenu = new QMenu("Time Line Interval", this);
	timeLineIntervalMenu->addAction(timeLineOffAction);
	timeLineIntervalMenu->addAction(setTimeLineIntervalAction);
	viewMenu->addMenu(timeLineIntervalMenu);

	// Construct Window menu.
	QMenu* windowMenu = menuBar()->addMenu("&Window");

	windowMenu->addAction(trackManagerDockWidget->toggleViewAction());
	windowMenu->addAction(eventManagerDockWidget->toggleViewAction());
	windowMenu->addAction(eventTypeManagerDockWidget->toggleViewAction());
	windowMenu->addAction(montageManagerDockWidget->toggleViewAction());
	windowMenu->addAction(filterManagerDockWidget->toggleViewAction());

	windowMenu->addSeparator();
	windowMenu->addAction(fileToolBar->toggleViewAction());
	windowMenu->addAction(filterToolBar->toggleViewAction());
	windowMenu->addAction(selectToolBar->toggleViewAction());
	windowMenu->addAction(zoomToolBar->toggleViewAction());
	windowMenu->addAction(spikedetToolBar->toggleViewAction());

	// Construct Tools menu.
	QMenu* toolsMenu = menuBar()->addMenu("&Tools");

	toolsMenu->addAction(runSpikedetAction);
	toolsMenu->addAction(spikedetSettingsAction);
	toolsMenu->addSeparator();

	toolsMenu->addAction(showSyncDialog);
	toolsMenu->addAction(synchronize);

	// Construct status bar.
	timeModeStatusLabel = new QLabel(this);
	timeModeStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	timeModeStatusLabel->addAction(timeModeAction0);
	timeModeStatusLabel->addAction(timeModeAction1);
	timeModeStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(timeModeStatusLabel);

	timeStatusLabel = new QLabel(this);
	timeStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	timeStatusLabel->addAction(timeModeAction0);
	timeStatusLabel->addAction(timeModeAction1);
	timeStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(timeStatusLabel);

	positionStatusLabel = new QLabel(this);
	positionStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	positionStatusLabel->addAction(timeModeAction0);
	positionStatusLabel->addAction(timeModeAction1);
	positionStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(positionStatusLabel);

	cursorStatusLabel = new QLabel(this);
	cursorStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	cursorStatusLabel->addAction(timeModeAction0);
	cursorStatusLabel->addAction(timeModeAction1);
	cursorStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(cursorStatusLabel);

	// Restore settings.
	restoreGeometry(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry").toByteArray());
	restoreState(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state").toByteArray());

	// Set up Spikedet.
	spikedetAnalysis = new SpikedetAnalysis(globalContext.get());

	auto settings = spikedetAnalysis->getSettings();
	SpikedetSettingsDialog::resetSettings(&settings, &spikeDuration, &originalDecimation);
	spikedetAnalysis->setSettings(settings);

	setEnableFileActions(false);
}

SignalFileBrowserWindow::~SignalFileBrowserWindow()
{
	closeFileDestroy();

	delete kernelCache;
	delete openDataFile;
	delete spikedetAnalysis;
	delete syncServer;
	delete syncClient;
}

QDateTime SignalFileBrowserWindow::sampleToDate(DataFile* file, int sample)
{
	int timeOffset = round(sample/file->getSamplingFrequency()*1000);

	QDateTime date = QDateTime::fromTime_t(file->getStartDate());
	date = date.addMSecs(timeOffset);

	return date;
}

QDateTime SignalFileBrowserWindow::sampleToOffset(DataFile* file, int sample)
{
	int timeOffset = round(sample/file->getSamplingFrequency()*1000);

	QDateTime date(QDate(1970, 1, 1));
	date = date.addMSecs(timeOffset);

	return date;
}

QString SignalFileBrowserWindow::sampleToDateTimeString(DataFile* file, int sample, InfoTable::TimeMode mode)
{
	QLocale locale;

	if (mode == InfoTable::TimeMode::size)
	{
		mode = OpenDataFile::infoTable.getTimeMode();
	}

	if (mode == InfoTable::TimeMode::samples)
	{
		return QString::number(sample);
	}
	else if (mode == InfoTable::TimeMode::offset)
	{
		QDateTime date = sampleToOffset(file, sample);
		return QString::number(date.date().day() - 1) + "d " + date.toString("hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
	}
	else if (mode == InfoTable::TimeMode::real)
	{
		return sampleToDate(file, sample).toString("d.M.yyyy hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
	}

	return QString();
}

void SignalFileBrowserWindow::closeEvent(QCloseEvent* event)
{
	if (closeFile())
	{
		SET_PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state", saveState());
		SET_PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry", saveGeometry());

		event->accept();
	}
	else
	{
		event->ignore();
	}
}

vector<QMetaObject::Connection> SignalFileBrowserWindow::connectVitness(const DataModelVitness* vitness, std::function<void ()> f)
{
	vector<QMetaObject::Connection> connections;

	auto c = connect(vitness, &DataModelVitness::valueChanged, f);
	connections.push_back(c);
	c = connect(vitness, &DataModelVitness::rowsInserted, f);
	connections.push_back(c);
	c = connect(vitness, &DataModelVitness::rowsRemoved, f);
	connections.push_back(c);

	return connections;
}

void SignalFileBrowserWindow::mode(int m)
{
	if (file)
	{
		OpenDataFile::infoTable.setTimeMode(static_cast<InfoTable::TimeMode>(m));

		updatePositionStatusLabel();
		updateCursorStatusLabel();
	}
}

bool SignalFileBrowserWindow::shouldSynchronizeView()
{
	return synchronize->isChecked();
}

void SignalFileBrowserWindow::deleteAutoSave()
{
	if (autoSaveName != "")
	{
		QFile autoSaveFile(autoSaveName.c_str());
		autoSaveFile.remove();
	}

	autoSaveTimer->start();
}

void SignalFileBrowserWindow::openFile()
{
	if (!closeFile())
		return; // User chose to keep open the current file.

	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Signal files (*.edf *.gdf *.mat);;EDF files (*.edf);;GDF files (*.gdf);;MAT files (*.mat)");

	if (fileName.isNull())
		return; // No file was selected.

	QFileInfo fileInfo(fileName);

	if (fileInfo.exists() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' not found.");
		return;
	}
	else if (fileInfo.isReadable() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' cannot be read.");
		return;
	}
	else if (fileInfo.isWritable() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' cannot be written to.");
		return;
	}

	logToFile("Opening file '" << fileName.toStdString() << "'.");

	setEnableFileActions(true);

	string stdFileName = fileName.toStdString();
	auto suffix = fileInfo.suffix().toLower();
	assert(!file);

	if (suffix == "gdf") // TODO: Add BDF support through Edflib.
	{
		file = new GDF2(stdFileName, PROGRAM_OPTIONS["uncalibratedGDF"].as<bool>());
	}
	else if (suffix == "edf")
	{
		file = new EDF(stdFileName);
	}
	else if (suffix == "mat")
	{
		file = new MAT(stdFileName);
	}
	else
	{
		throw runtime_error("Unknown file extension.");
	}

	dataModel = new DataModel(new VitnessEventTypeTable(), new VitnessMontageTable());
	file->setDataModel(dataModel);

	undoFactory = new UndoCommandFactory(dataModel, undoStack);

	openDataFile->file = file;
	openDataFile->dataModel = dataModel;
	openDataFile->undoFactory = undoFactory;
	openDataFile->kernelCache = kernelCache;

	autoSaveName = file->getFilePath() + ".mont.autosave";
	bool useAutoSave = false;

	if (QFileInfo(autoSaveName.c_str()).exists())
	{
		auto res = QMessageBox::question(this,
			"Load Autosave File?", "An autosave file was detected. Would you like to load it?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

		useAutoSave = res == QMessageBox::Yes;
	}

	bool secondaryFileExists;
	executeWithCLocale([this, useAutoSave, &secondaryFileExists] () {
		if (useAutoSave)
			secondaryFileExists = file->loadSecondaryFile(autoSaveName);
		else
			secondaryFileExists = file->load();

		AlenkaSignal::DETECTOR_SETTINGS settings;
		OpenDataFile::infoTable.readXML(file->getFilePath() + ".info", &settings, &spikeDuration, &originalDecimation);
	});

	if (useAutoSave || !secondaryFileExists)
	{
		saveFileAction->setEnabled(true); // Allow save when the secondary file can be created or is out of sync with the autosave.
		allowSaveOnClean = true;
	}
	else
	{
		allowSaveOnClean = false;
	}
	cleanChanged(undoStack->isClean());

	setWindowTitle(fileInfo.fileName() + " - " + TITLE);

	// Check for any values in InfoTable that could make trouble.
	if (OpenDataFile::infoTable.getSelectedMontage() < 0 || OpenDataFile::infoTable.getSelectedMontage() >= openDataFile->dataModel->montageTable()->rowCount())
		OpenDataFile::infoTable.setSelectedMontage(0);

	// Pass the file to the child widgets.
	trackManager->changeFile(openDataFile);
	eventManager->changeFile(openDataFile);
	eventTypeManager->changeFile(openDataFile);
	montageManager->changeFile(openDataFile);
	filterManager->changeFile(openDataFile);

	signalViewer->changeFile(openDataFile);

	// Update Filter tool bar.
	vector<double> comboNumbers{0, 5, 10};
	for (int i = 25; i <= file->getSamplingFrequency()/2; i *= 2)
		comboNumbers.push_back(i);

	double lpf = OpenDataFile::infoTable.getLowpassFrequency();
	bool lowpassOn = OpenDataFile::infoTable.getLowpassOn();
	if (lowpassOn && 0 < lpf && lpf <= file->getSamplingFrequency()/2)
		comboNumbers.push_back(lpf);

	double hpf = OpenDataFile::infoTable.getHighpassFrequency();
	bool highpassOn = OpenDataFile::infoTable.getHighpassOn();
	if (highpassOn && 0 < hpf && hpf <= file->getSamplingFrequency()/2)
		comboNumbers.push_back(hpf);

	sort(comboNumbers.begin(), comboNumbers.end());
	comboNumbers.erase(unique(comboNumbers.begin(), comboNumbers.end()), comboNumbers.end());

	QStringList comboOptions("---");
	for (double e : comboNumbers)
		comboOptions << locale().toString(e, 'f', 2);

	QMetaObject::Connection c;

	lowpassComboBox->clear();
	lowpassComboBox->addItems(comboOptions);
	c = connect(lowpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(lowpassComboBoxUpdate(QString)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(lowpassComboBoxUpdate(double)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassOnChanged(bool)), this, SLOT(lowpassComboBoxUpdate(bool)));
	openFileConnections.push_back(c);

	highpassComboBox->clear();
	highpassComboBox->addItems(comboOptions);
	c = connect(highpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(highpassComboBoxUpdate(QString)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(highpassComboBoxUpdate(double)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassOnChanged(bool)), this, SLOT(highpassComboBoxUpdate(bool)));
	openFileConnections.push_back(c);

	c = connect(notchCheckBox, SIGNAL(toggled(bool)), &OpenDataFile::infoTable, SLOT(setNotchOn(bool)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(notchOnChanged(bool)), notchCheckBox, SLOT(setChecked(bool)));
	openFileConnections.push_back(c);

	// Set up table models and managers.
	eventTypeTable = new EventTypeTableModel(openDataFile);
	eventTypeManager->setModel(eventTypeTable);

	montageTable = new MontageTableModel(openDataFile);
	montageManager->setModel(montageTable);

	eventTable = new EventTableModel(openDataFile);
	eventManager->setModel(eventTable);

	trackTable = new TrackTableModel(openDataFile);
	trackManager->setModel(trackTable);

	// Update the Select tool bar.
	auto cc = connectVitness(VitnessMontageTable::vitness(dataModel->montageTable()), [this] () { updateMontageComboBox(); });
	openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
	updateMontageComboBox();

	c = connect(montageComboBox, SIGNAL(currentIndexChanged(int)), &OpenDataFile::infoTable, SLOT(setSelectedMontage(int)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), montageComboBox, SLOT(setCurrentIndex(int)));
	openFileConnections.push_back(c);

	cc = connectVitness(VitnessEventTypeTable::vitness(dataModel->eventTypeTable()), [this] () { updateEventTypeComboBox(); });
	openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
	updateEventTypeComboBox();

	c = connect(eventTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [] (int index) {
		OpenDataFile::infoTable.setSelectedType(index - 1);
	});
	openFileConnections.push_back(c);

	c = connect(&OpenDataFile::infoTable, &InfoTable::selectedTypeChanged, [this] (int value) {
		eventTypeComboBox->setCurrentIndex(value + 1);
	});
	openFileConnections.push_back(c);

	vector<float> resolutionNumbers{1, 2, 3, 6, 12};
	float sampleScaleValue = OpenDataFile::infoTable.getSampleScale();
	resolutionNumbers.push_back(sampleScaleValue);
	sort(resolutionNumbers.begin(), resolutionNumbers.end());
	resolutionNumbers.erase(unique(resolutionNumbers.begin(), resolutionNumbers.end()), resolutionNumbers.end());

	QStringList resolutionOptions;
	for (double e : resolutionNumbers)
		resolutionOptions << locale().toString(e, 'f', 2);
	resolutionComboBox->clear();
	resolutionComboBox->addItems(resolutionOptions);

	c = connect(resolutionComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(resolutionComboBoxUpdate(QString)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(sampleScaleChanged(float)), this, SLOT(resolutionComboBoxUpdate(float)));
	openFileConnections.push_back(c);

	// Update the status bar.
	QString str = "Start: " + sampleToDateTimeString(file, 0, InfoTable::TimeMode::real);
	str += " Total time: " + sampleToDateTimeString(file, file->getSamplesRecorded(), InfoTable::TimeMode::offset);
	timeStatusLabel->setText(str);

	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this, SLOT(updatePositionStatusLabel()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), this, SLOT(updatePositionStatusLabel()));
	openFileConnections.push_back(c);
	c = connect(signalViewer->getCanvas(), SIGNAL(cursorPositionSampleChanged(int)), this, SLOT(updateCursorStatusLabel()));
	openFileConnections.push_back(c);

	// Connect slot SignalViewer::updateSignalViewer() to make sure that the SignalViewer gets updated when needed.
	// TODO: Perhaps move this block to signalViewer.
	c = connect(&OpenDataFile::infoTable, SIGNAL(virtualWidthChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassOnChanged(bool)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassFrequencyChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassOnChanged(bool)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(notchOnChanged(bool)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(filterWindowChanged(AlenkaSignal::WindowFunction)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(timeLineIntervalChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersChanged()), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersOnChanged(bool)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(sampleScaleChanged(float)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(sampleUnitsChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);

	cc = connectVitness(VitnessMontageTable::vitness(dataModel->montageTable()), [this] () { signalViewer->updateSignalViewer(); });
	openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
	cc = connectVitness(VitnessEventTypeTable::vitness(dataModel->eventTypeTable()), [this] () { signalViewer->updateSignalViewer(); });
	openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateManagers(int)));
	openFileConnections.push_back(c);

	// Update the View submenus.
	c = connect(&OpenDataFile::infoTable, SIGNAL(timeModeChanged(InfoTable::TimeMode)), this, SLOT(updateTimeMode(InfoTable::TimeMode)));
	c = connect(&OpenDataFile::infoTable, &InfoTable::timeLineIntervalChanged, [this] (double value) {
		setTimeLineIntervalAction->setToolTip("The time line interval is " + locale().toString(value) + " s.");
		setTimeLineIntervalAction->setStatusTip(setTimeLineIntervalAction->toolTip());
	});
	openFileConnections.push_back(c);

	// Connect Sync.
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this, SLOT(sendSyncMessage()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), this, SLOT(sendSyncMessage()));
	openFileConnections.push_back(c);

	// Emit all signals to ensure there are no uninitialized controls.
	OpenDataFile::infoTable.emitAllSignals();

	// Set up autosave.
	c = connect(autoSaveTimer, &QTimer::timeout, [this] () {
		if (undoStack->isClean())
			return;

		executeWithCLocale([this] () {
			file->saveSecondaryFile(autoSaveName);
			logToFileAndConsole("Autosaving to " << autoSaveName);
		});
	});
	openFileConnections.push_back(c);

	int ms = 1000*PROGRAM_OPTIONS["autosaveInterval"].as<int>();
	autoSaveTimer->setInterval(ms);
	autoSaveTimer->start();
}

bool SignalFileBrowserWindow::closeFile()
{
	if (!undoStack->isClean())
	{
		auto res = QMessageBox::question(this, "Save File?", "Save changes before closing?", QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);

		if (res == QMessageBox::Save)
			saveFile();
		else if (res == QMessageBox::Cancel)
			return false;
	}

	logToFile("Closing file.");

	setWindowTitle(TITLE);
	undoStack->clear();
	setEnableFileActions(false);

	if (file)
	{
		executeWithCLocale([this] () {
			OpenDataFile::infoTable.writeXML(file->getFilePath() + ".info", spikedetAnalysis->getSettings(), spikeDuration, originalDecimation);
		});
	}

	OpenDataFile::infoTable.setFilterCoefficients(vector<float>());

	deleteAutoSave();
	autoSaveName = "";

	closeFileDestroy();

	signalViewer->updateSignalViewer();

	return true;
}

void SignalFileBrowserWindow::saveFile()
{
	logToFile("Saving file.");

	if (file)
	{
		executeWithCLocale([this] () {
			file->save();
		});

		deleteAutoSave();

		allowSaveOnClean = false;
		undoStack->setClean();
		saveFileAction->setEnabled(false);
		autoSaveTimer->start();
	}
}

void SignalFileBrowserWindow::exportToEdf()
{
	assert(file);

	QFileInfo fileInfo(QString::fromStdString(file->getFilePath()));
	QString fileName = QFileDialog::getSaveFileName(this, "Export to EDF file", fileInfo.dir().absolutePath(), "EDF files (*.edf)");

	if (fileName.isNull())
		return; // No file was selected.

	QFileInfo newFileInfo(fileName);
	if (newFileInfo.suffix() != "edf")
		fileName += ".edf";

	EDF::saveAs(fileName.toStdString(), file);
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(const QString& text)
{
	if (file)
	{
		bool ok;
		double tmp = locale().toDouble(text, &ok);

		if (ok)
			OpenDataFile::infoTable.setLowpassFrequency(tmp);
		OpenDataFile::infoTable.setLowpassOn(ok);
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(bool on)
{
	if (file)
	{
		if (on)
			lowpassComboBoxUpdate(OpenDataFile::infoTable.getLowpassFrequency());
		else
			lowpassComboBox->setCurrentIndex(0);
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(double value)
{
	if (file)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
			lowpassComboBoxUpdate(false);
		else
			lowpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(const QString& text)
{
	if (file)
	{
		bool ok;
		double tmp = locale().toDouble(text, &ok);

		if (ok)
			OpenDataFile::infoTable.setHighpassFrequency(tmp);
		OpenDataFile::infoTable.setHighpassOn(ok);
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(bool on)
{
	if (file)
	{
		if (on)
			highpassComboBoxUpdate(OpenDataFile::infoTable.getHighpassFrequency());
		else
			highpassComboBox->setCurrentIndex(0);
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(double value)
{
	if (file)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
			highpassComboBoxUpdate(false);
		else
			highpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
	}
}

void SignalFileBrowserWindow::resolutionComboBoxUpdate(const QString& text)
{
	if (file)
	{
		bool ok;
		float tmp = locale().toFloat(text, &ok);

		if (ok)
			OpenDataFile::infoTable.setSampleScale(tmp);
	}
}

void SignalFileBrowserWindow::resolutionComboBoxUpdate(float value)
{
	if (file)
	{
		resolutionComboBox->setCurrentText(locale().toString(value, 'f', 2));
	}
}

void SignalFileBrowserWindow::updateManagers(int value)
{
	for (auto e : managersConnections)
		disconnect(e);
	managersConnections.clear();

	if (0 < dataModel->montageTable()->rowCount())
	{
		auto cc = connectVitness(VitnessTrackTable::vitness(dataModel->montageTable()->trackTable(value)), [this] () { signalViewer->updateSignalViewer(); });
		managersConnections.insert(managersConnections.end(), cc.begin(), cc.end());
		cc = connectVitness(VitnessEventTable::vitness(dataModel->montageTable()->eventTable(value)), [this] () { signalViewer->updateSignalViewer(); });
		managersConnections.insert(managersConnections.end(), cc.begin(), cc.end());
	}
}

void SignalFileBrowserWindow::updateTimeMode(InfoTable::TimeMode mode)
{
	QAction* a = timeModeActionGroup->actions().at(static_cast<int>(mode));
	a->setChecked(true);

	timeModeStatusLabel->setText("Time Mode: " + a->text());
}

void SignalFileBrowserWindow::updatePositionStatusLabel()
{
	double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
	double position = OpenDataFile::infoTable.getPosition() + OpenDataFile::infoTable.getPixelViewWidth()*OpenDataFile::infoTable.getPositionIndicator();

	positionStatusLabel->setText("Position: " + sampleToDateTimeString(file, round(position*ratio)));
}

void SignalFileBrowserWindow::updateCursorStatusLabel()
{
	cursorStatusLabel->setText("Cursor at: " + sampleToDateTimeString(file, signalViewer->getCanvas()->getCursorPositionSample()));
}

void SignalFileBrowserWindow::updateMontageComboBox()
{
	if (file)
	{
		const AbstractMontageTable* montageTable = openDataFile->dataModel->montageTable();
		int itemCount = montageComboBox->count();
		int selectedMontage = max(OpenDataFile::infoTable.getSelectedMontage(), 0);

		for (int i = 0; i < montageTable->rowCount(); ++i)
			montageComboBox->addItem(QString::fromStdString(montageTable->row(i).name));

		for (int i = 0; i < itemCount; ++i)
			montageComboBox->removeItem(0);

		OpenDataFile::infoTable.setSelectedMontage(min(selectedMontage, montageTable->rowCount() - 1));
	}
}

void SignalFileBrowserWindow::updateEventTypeComboBox()
{
	if (file)
	{
		const AbstractEventTypeTable* eventTypeTable = openDataFile->dataModel->eventTypeTable();
		int itemCount = eventTypeComboBox->count();
		int selectedType = OpenDataFile::infoTable.getSelectedType();

		eventTypeComboBox->addItem("<No Type>");
		for (int i = 0; i < eventTypeTable->rowCount(); ++i)
			eventTypeComboBox->addItem(QString::fromStdString(eventTypeTable->row(i).name));

		for (int i = 0; i < itemCount; ++i)
			eventTypeComboBox->removeItem(0);

		OpenDataFile::infoTable.setSelectedType(min(selectedType, eventTypeTable->rowCount() - 1));
	}
}

void SignalFileBrowserWindow::runSpikedet()
{
	if (!file)
		return;

	const AbstractMontageTable* montageTable = openDataFile->dataModel->montageTable();
	if (montageTable->rowCount() <= 0)
		return;

	const AbstractTrackTable* trackTable = montageTable->trackTable(OpenDataFile::infoTable.getSelectedMontage());
	if (trackTable->rowCount() <= 0)
		return;

	undoFactory->beginMacro("run Spikedet");

	// Build montage from code.
	QFile headerFile(":/montageHeader.cl");
	headerFile.open(QIODevice::ReadOnly);
	string header = headerFile.readAll().toStdString();

	vector<string> montageCode;
	for (int i = 0; i < trackTable->rowCount(); ++i)
		montageCode.push_back(trackTable->row(i).code);

	auto montage = SignalProcessor::makeMontage(montageCode, globalContext.get(), kernelCache, header);

	// Run Spikedet.
	QProgressDialog progress("Running Spikedet analysis", "Abort", 0, 100, this);
	progress.setWindowModality(Qt::WindowModal);

	progress.setMinimumDuration(0); // This is to show the dialog immediately.
	progress.setValue(1);

	spikedetAnalysis->runAnalysis(openDataFile, montage, &progress, originalDecimation);

	// Add three new event types for the different levels of spike events.
	const AbstractEventTypeTable* eventTypeTable = openDataFile->dataModel->eventTypeTable();
	int index = eventTypeTable->rowCount();
	undoFactory->insertEventType(index, 3);

	QColor colors[3] = {QColor(0, 0, 255), QColor(0, 255, 0), QColor(0, 255, 255)};
	for (int i = 0; i < 3; ++i)
	{
		EventType et = eventTypeTable->row(index + i);

		et.name = "Spikedet K" + to_string(i + 1);
		DataModel::color2array(colors[i], et.color);

		undoFactory->changeEventType(index + i, et);
	}

	// Process the output structure.
	const AbstractEventTable* eventTable = montageTable->eventTable(OpenDataFile::infoTable.getSelectedMontage());

	AlenkaSignal::CDetectorOutput* out = spikedetAnalysis->getOutput();
	assert(out);
	int count = static_cast<int>(out->m_pos.size());

	if (count > 0)
	{
		assert(static_cast<int>(out->m_chan.size()) == count);

		int etIndex = eventTable->rowCount();
		undoFactory->insertEvent(OpenDataFile::infoTable.getSelectedMontage(), etIndex, count);

		for (int i = 0; i < count; i++)
		{
			Event e = eventTable->row(etIndex + i);

			e.label = "Spike " + to_string(i);
			e.type = index + (out->m_con[i] == 0.5 ? 1 : 0); // TODO: Figure out what should be used as the third type.
			e.position = out->m_pos[i]*file->getSamplingFrequency();
			e.duration = file->getSamplingFrequency()*spikeDuration;
			//e.duration = out->m_dur[i]*file->getSamplingFrequency();
			e.channel = out->m_chan[i] - 1;

			undoFactory->changeEvent(OpenDataFile::infoTable.getSelectedMontage(), etIndex + i, e);
		}
	}

	undoFactory->endMacro();
}

void SignalFileBrowserWindow::receiveSyncMessage(const QByteArray& message)
{
	if (file && shouldSynchronizeView())
	{
		double timePosition;
		unpackMessage(message, &timePosition);

		const double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
		int position = timePosition*file->getSamplingFrequency()/ratio;

#ifndef NDEBUG
		cerr << "Received position: " << position << " " << timePosition << endl;
#endif

		lastPositionReceived = position;
		position -= OpenDataFile::infoTable.getPixelViewWidth()*OpenDataFile::infoTable.getPositionIndicator();
		OpenDataFile::infoTable.setPosition(position);
	}
}

void SignalFileBrowserWindow::sendSyncMessage()
{
	if (file && shouldSynchronizeView())
	{
		const int position = OpenDataFile::infoTable.getPosition() + OpenDataFile::infoTable.getPixelViewWidth()*OpenDataFile::infoTable.getPositionIndicator();

		const double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
		const int epsilon = max<int>(3, file->getSamplingFrequency()/50/ratio);

		// This condition is to break the message deadlock. This comes about because the source of the signal that trigers
		// this method cannot be distinguished between user input and received sync message.

		// When the current position is very close (within a small fraction of a second) to the last received position,
		// you are most likely just repeating what the server already has. So there is no need to send this information.
		if (position < (lastPositionReceived - epsilon) || position > (lastPositionReceived + epsilon))
		{
			const double timePosition = position*ratio/file->getSamplingFrequency();
#ifndef NDEBUG
			if (syncServer->connectionCount() > 0 || syncClient->isValid())
				cerr << "Sending position: " << position << " " << timePosition << endl;
#endif
			QByteArray message = packMessage(timePosition);

			syncServer->sendMessage(message);
			syncClient->sendMessage(message);
		}
#ifndef NDEBUG
		else
		{
			cerr << "Message skipped: " << position << endl;
		}
#endif

		// Reset to the default value, so that the message is skipped at most once.
		lastPositionReceived = lastPositionReceivedDefault;
	}
}

void SignalFileBrowserWindow::cleanChanged(bool clean)
{
	if (clean && allowSaveOnClean)
		saveFileAction->setEnabled(true);
	else
		saveFileAction->setEnabled(!clean);
}

void SignalFileBrowserWindow::closeFileDestroy()
{
	for (auto e : openFileConnections)
		disconnect(e);
	openFileConnections.clear();

	montageComboBox->clear();
	eventTypeComboBox->clear();

	signalViewer->changeFile(nullptr);

	trackManager->changeFile(nullptr);
	eventManager->changeFile(nullptr);
	eventTypeManager->changeFile(nullptr);
	montageManager->changeFile(nullptr);
	filterManager->changeFile(nullptr);

	delete eventTypeTable; eventTypeTable = nullptr;
	delete montageTable; montageTable = nullptr;
	delete eventTable; eventTable = nullptr;
	delete trackTable; trackTable = nullptr;

	delete dataModel; dataModel = nullptr;

	delete undoFactory; undoFactory = nullptr;
	delete file; file = nullptr;
}

void SignalFileBrowserWindow::setEnableFileActions(bool enable)
{
	closeFileAction->setEnabled(enable);
	runSpikedetAction->setEnabled(enable);
	exportToEdfAction->setEnabled(enable);
}
