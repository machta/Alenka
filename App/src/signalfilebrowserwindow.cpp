#include "signalfilebrowserwindow.h"

#include "options.h"
#include "signalviewer.h"
#include "DataFile/gdf2.h"
#include "DataFile/edftmp.h"
#include "Manager/trackmanager.h"
#include "Manager/eventmanager.h"
#include "Manager/eventtypemanager.h"
#include "Manager/montagemanager.h"
#include "spikedetanalysis.h"
#include "myapplication.h"
#include "spikedetsettingsdialog.h"

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>

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

using namespace std;

namespace
{
const double horizontalZoomFactor = 1.3;
const double verticalZoomFactor = 1.3;

const char* title = "Signal File Browser";
}

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(title);

	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

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

	QDockWidget* eventTypeDockWidget = new QDockWidget("EventType Manager", this);
	eventTypeDockWidget->setObjectName("EventType Manager QDockWidget");
	eventTypeManager = new EventTypeManager(this);
	eventTypeDockWidget->setWidget(eventTypeManager);

	QDockWidget* montageManagerDockWidget = new QDockWidget("Montage Manager", this);
	montageManagerDockWidget->setObjectName("Montage Manager QDockWidget");
	montageManager = new MontageManager(this);
	montageManagerDockWidget->setWidget(montageManager);

	addDockWidget(Qt::RightDockWidgetArea, trackManagerDockWidget);
	tabifyDockWidget(trackManagerDockWidget, eventManagerDockWidget);
	tabifyDockWidget(eventManagerDockWidget, eventTypeDockWidget);
	tabifyDockWidget(eventTypeDockWidget, montageManagerDockWidget);

	// Construct File actions.
	QAction* openFileAction = new QAction("&Open File", this);
	openFileAction->setShortcut(QKeySequence::Open);
	openFileAction->setToolTip("Open an existing file.");
	openFileAction->setStatusTip(openFileAction->toolTip());
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	QAction* closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcut(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	closeFileAction->setStatusTip(closeFileAction->toolTip());
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	QAction* saveFileAction = new QAction("Save File", this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveFileAction->setToolTip("Save the currently opened file.");
	saveFileAction->setStatusTip(saveFileAction->toolTip());
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	// Construct Zoom actions.
	QAction* horizontalZoomInAction = new QAction("Horizontal Zoom In", this);
	horizontalZoomInAction->setShortcut(QKeySequence("Alt++"));
	horizontalZoomInAction->setToolTip("Zoom in time line.");
	horizontalZoomInAction->setStatusTip(horizontalZoomInAction->toolTip());
	connect(horizontalZoomInAction, SIGNAL(triggered()), this, SLOT(horizontalZoomIn()));

	QAction* horizontalZoomOutAction = new QAction("Horizontal Zoom Out", this);
	horizontalZoomOutAction->setShortcut(QKeySequence("Alt+-"));
	horizontalZoomOutAction->setToolTip("Zoom out time line.");
	horizontalZoomOutAction->setStatusTip(horizontalZoomOutAction->toolTip());
	connect(horizontalZoomOutAction, SIGNAL(triggered()), this, SLOT(horizontalZoomOut()));

	QAction* verticalZoomInAction = new QAction("Vertical Zoom In", this);
	verticalZoomInAction->setShortcut(QKeySequence("Shift++"));
	verticalZoomInAction->setToolTip("Zoom in amplitudes of signals.");
	verticalZoomInAction->setStatusTip(verticalZoomInAction->toolTip());
	connect(verticalZoomInAction, SIGNAL(triggered()), this, SLOT(verticalZoomIn()));

	QAction* verticalZoomOutAction = new QAction("Vertical Zoom Out", this);
	verticalZoomOutAction->setShortcut(QKeySequence("Shift+-"));
	verticalZoomOutAction->setToolTip("Zoom out amplitudes of signals.");
	verticalZoomOutAction->setStatusTip(verticalZoomOutAction->toolTip());
	connect(verticalZoomOutAction, SIGNAL(triggered()), this, SLOT(verticalZoomOut()));

	// Construct Spikedet actions.
	QAction* runSpikedetAction = new QAction("Run Spikedet Analysis", this);
	runSpikedetAction->setToolTip("Run Spikedet analysis on the current montage.");
	runSpikedetAction->setStatusTip(runSpikedetAction->toolTip());
	connect(runSpikedetAction, &QAction::triggered, [this] () { runSpikedet(); } );

	QAction* spikedetSettingsAction = new QAction("Spikedet Settings", this);
	spikedetSettingsAction->setToolTip("Change Spikedet settings.");
	spikedetSettingsAction->setStatusTip(spikedetSettingsAction->toolTip());
	connect(spikedetSettingsAction, &QAction::triggered, [this] ()
	{
		AlenkaSignal::DETECTOR_SETTINGS settings = spikedetAnalysis->getSettings();
		SpikedetSettingsDialog dialog(&settings, this);

		if (dialog.exec() == QDialog::Accepted)
			spikedetAnalysis->setSettings(settings);
	});

	// Construct Time Mode action group.
	timeModeActionGroup = new QActionGroup(this);

	QAction* timeModeAction0 = new QAction("Sample", this);
	timeModeAction0->setToolTip("Samples from the start.");
	timeModeAction0->setStatusTip(timeModeAction0->toolTip());
	connect(timeModeAction0, SIGNAL(triggered()), this, SLOT(timeMode0()));
	timeModeAction0->setActionGroup(timeModeActionGroup);
	timeModeAction0->setCheckable(true);

	QAction* timeModeAction1 = new QAction("Offset", this);
	timeModeAction1->setToolTip("Time offset from the start.");
	timeModeAction1->setStatusTip(timeModeAction1->toolTip());
	connect(timeModeAction1, SIGNAL(triggered()), this, SLOT(timeMode1()));
	timeModeAction1->setActionGroup(timeModeActionGroup);
	timeModeAction1->setCheckable(true);

	QAction* timeModeAction2 = new QAction("Real", this);
	timeModeAction2->setToolTip("Real time.");
	timeModeAction2->setStatusTip(timeModeAction2->toolTip());
	connect(timeModeAction2, SIGNAL(triggered()), this, SLOT(timeMode2()));
	timeModeAction2->setActionGroup(timeModeActionGroup);
	timeModeAction2->setCheckable(true);

	// Construct Time Line Interval action group.
	timeLineIntervalActionGroup = new QActionGroup(this);

	QAction* timeLineOffAction = new QAction("Off", this);
	timeLineOffAction->setToolTip("Turn off the time lines.");
	timeLineOffAction->setStatusTip(timeLineOffAction->toolTip());
	connect(timeLineOffAction, &QAction::triggered, [this] ()
	{
		if (file != nullptr)
		{
			file->getInfoTable()->setTimeLineInterval(0);
		}
	});

	setTimeLineIntervalAction = new QAction("Set", this);
	setTimeLineIntervalAction->setActionGroup(timeLineIntervalActionGroup);
	connect(setTimeLineIntervalAction, &QAction::triggered, [this] ()
	{
		if (file != nullptr)
		{
			double value = file->getInfoTable()->getTimeLineInterval();
			if (value == 0)
			{
				value = 1;
			}

			bool ok;
			value = QInputDialog::getDouble(this, "Set the interval", "Please, enter the value for the time line interval here:", value, 0, 1000*1000*1000, 2, &ok);

			if (ok)
			{
				file->getInfoTable()->setTimeLineInterval(value);
			}
		}
	});

	// Tool bars.
	const int spacing = 3;

	// Construct File tool bar.
	QToolBar* fileToolBar = addToolBar("File Tool Bar");
	fileToolBar->setObjectName("File QToolBar");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

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

	selectToolBar->addWidget(new QLabel("Montage:", this));
	montageComboBox = new QComboBox(this);
	montageComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	montageComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(montageComboBox);

	selectToolBar->addWidget(new QLabel("Event Type:", this));
	eventTypeComboBox = new QComboBox(this);
	eventTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	eventTypeComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(eventTypeComboBox);

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

	// Construct View menu.
	QMenu* viewMenu = menuBar()->addMenu("&View");

	viewMenu->addAction(horizontalZoomInAction);
	viewMenu->addAction(horizontalZoomOutAction);
	viewMenu->addAction(verticalZoomInAction);
	viewMenu->addAction(verticalZoomOutAction);

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
	windowMenu->addAction(eventTypeDockWidget->toggleViewAction());
	windowMenu->addAction(montageManagerDockWidget->toggleViewAction());

	windowMenu->addSeparator();
	windowMenu->addAction(fileToolBar->toggleViewAction());
	windowMenu->addAction(filterToolBar->toggleViewAction());
	windowMenu->addAction(selectToolBar->toggleViewAction());
	windowMenu->addAction(zoomToolBar->toggleViewAction());
	windowMenu->addAction(spikedetToolBar->toggleViewAction());

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
}

SignalFileBrowserWindow::~SignalFileBrowserWindow()
{
	delete file;
}

void SignalFileBrowserWindow::closeEvent(QCloseEvent* event)
{
	// Store settings.
	PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry", saveGeometry());
	PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state", saveState());

	QMainWindow::closeEvent(event);
}

void SignalFileBrowserWindow::connectModel(QAbstractTableModel* model, std::function<void ()> f)
{
	connect(model, &QAbstractTableModel::dataChanged, f);
	connect(model, &QAbstractTableModel::rowsInserted, f);
	connect(model, &QAbstractTableModel::rowsRemoved, f);
}

void SignalFileBrowserWindow::horizontalZoom(double factor)
{
	if (file != nullptr)
	{
		InfoTable* it = file->getInfoTable();
		it->setVirtualWidth(it->getVirtualWidth()*factor);
	}
}

void SignalFileBrowserWindow::verticalZoom(double factor)
{
	if (file != nullptr)
	{
		TrackTable* tt = file->getMontageTable()->getTrackTables()->at(file->getInfoTable()->getSelectedMontage());

		for (int i = 0; i < tt->rowCount(); ++i)
		{
			double value = tt->getAmplitude(i)*factor;
			value = value != 0 ? value : -0.000001;
			tt->setAmplitude(value, i);
		}

		emit tt->dataChanged(tt->index(0, static_cast<int>(TrackTable::Column::amplitude)), tt->index(tt->rowCount() - 1, static_cast<int>(TrackTable::Column::amplitude)));
	}
}

void SignalFileBrowserWindow::mode(int m)
{
	if (file != nullptr)
	{
		InfoTable* it = file->getInfoTable();
		EventTable* et = file->getMontageTable()->getEventTables()->at(it->getSelectedMontage());

		it->setTimeMode(static_cast<InfoTable::TimeMode>(m));

		et->emitColumnChanged(EventTable::Column::position);
		et->emitColumnChanged(EventTable::Column::duration);

		updatePositionStatusLabel();
		updateCursorStatusLabel();
	}
}

void SignalFileBrowserWindow::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "GDF file (*.gdf);;EDF file (*.edf)");

	logToFile("Opening file '" << fileName.toStdString() << "'.");

	if (fileName.isNull())
	{
		// No file was selected.
		return;
	}

	// Open the file.
	delete file;

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

	string stdFileName = (fileInfo.path() + QDir::separator() + fileInfo.completeBaseName()).toStdString();
	if (fileInfo.suffix().toLower() == "gdf")
	{
		file = new GDF2(stdFileName);
	}
	else
	{
		file = new EdfTmp(stdFileName);
	}

	setWindowTitle(fileInfo.fileName() + " - " + title);

	InfoTable* it = file->getInfoTable();

	// Check for any values in InfoTable that could make trouble.
	if (it->getSelectedMontage() < 0 || it->getSelectedMontage() >= file->getMontageTable()->rowCount())
	{
		it->setSelectedMontage(0);
	}

	// Pass the file to the child widgets.
	signalViewer->changeFile(file);
	eventManager->changeFile(file);

	// Update Filter tool bar.
	QStringList comboOptions;

	comboOptions << "---" << "0" << "5" << "10";

	for (int i = 50; i <= file->getSamplingFrequency()/2; i *= 2)
	{
		comboOptions << QString::number(i);
	}

	lowpassComboBox->clear();
	lowpassComboBox->addItems(comboOptions);
	connect(lowpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(lowpassComboBoxUpdate(QString)));
	connect(it, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(lowpassComboBoxUpdate(double)));

	highpassComboBox->clear();
	highpassComboBox->addItems(comboOptions);
	connect(highpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(highpassComboBoxUpdate(QString)));
	connect(it, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(highpassComboBoxUpdate(double)));

	connect(notchCheckBox, SIGNAL(toggled(bool)), it, SLOT(setNotch(bool)));
	connect(it, SIGNAL(notchChanged(bool)), notchCheckBox, SLOT(setChecked(bool)));

	// Update the Select tool bar.
	connectModel(file->getMontageTable(), [this] () { updateMontageComboBox(); });
	updateMontageComboBox();
	connect(montageComboBox, SIGNAL(currentIndexChanged(int)), it, SLOT(setSelectedMontage(int)));
	connect(it, SIGNAL(selectedMontageChanged(int)), montageComboBox, SLOT(setCurrentIndex(int)));

	connectModel(file->getEventTypeTable(), [this] () { updateEventTypeComboBox(); });
	updateEventTypeComboBox();
	connect(eventTypeComboBox, SIGNAL(currentIndexChanged(int)), it, SLOT(setSelectedType(int)));
	connect(it, SIGNAL(selectedTypeChanged(int)), eventTypeComboBox, SLOT(setCurrentIndex(int)));

	// Update the managers.
	eventTypeManager->setModel(file->getEventTypeTable());
	montageManager->setModel(file->getMontageTable());

	connect(it, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateManagers(int)));

	// Update the status bar.
	timeStatusLabel->setText("Start: " + file->sampleToDateTimeString(0, InfoTable::TimeMode::real) + " Total time: " + file->sampleToDateTimeString(file->getSamplesRecorded(), InfoTable::TimeMode::offset));

	connect(it, SIGNAL(positionChanged(int)), this, SLOT(updatePositionStatusLabel()));
	connect(it, SIGNAL(positionIndicatorChanged(double)), this, SLOT(updatePositionStatusLabel()));
	connect(signalViewer->getCanvas(), SIGNAL(cursorPositionSampleChanged(int)), this, SLOT(updateCursorStatusLabel()));

	// Connect slot SignalViewer::update() to make sure that the SignalViewer gets updated when needed.
	connect(it, SIGNAL(virtualWidthChanged(int)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(positionChanged(int)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(lowpassFrequencyChanged(double)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(highpassFrequencyChanged(double)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(notchChanged(bool)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(selectedMontageChanged(int)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(timeLineIntervalChanged(double)), signalViewer, SLOT(update()));
	connect(it, SIGNAL(positionIndicatorChanged(double)), signalViewer, SLOT(update()));

	connectModel(file->getMontageTable(), [this] () { signalViewer->update(); });
	connectModel(file->getEventTypeTable(), [this] () { signalViewer->update(); });

	// Update the View submenus.
	connect(it, SIGNAL(timeModeChanged(int)), this, SLOT(updateTimeMode(int)));
	connect(it, &InfoTable::timeLineIntervalChanged, [this] (double value)
	{
		setTimeLineIntervalAction->setToolTip("The time line interval is " + locale().toString(value) + " s.");
		setTimeLineIntervalAction->setStatusTip(setTimeLineIntervalAction->toolTip());
	});

	// Emit all signals to ensure there are no uninitialized controls.
	it->emitAllSignals();
}

void SignalFileBrowserWindow::closeFile()
{
	logToFile("Closing file.");

	setWindowTitle(title);

	delete file;
	file = nullptr;

	signalViewer->changeFile(nullptr);
	eventManager->changeFile(nullptr);

	signalViewer->update();
}

void SignalFileBrowserWindow::saveFile()
{
	logToFile("Saving file.");

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
		double tmp = locale().toDouble(text, &ok);

		if (ok)
		{
			file->getInfoTable()->setLowpassFrequency(tmp);
		}
		else
		{
			file->getInfoTable()->setLowpassFrequency(1000000); // TODO: properly turn of/on
		}
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(double value)
{
	if (file != nullptr)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
		{
			lowpassComboBox->setCurrentIndex(0);
		}
		else
		{
			lowpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
		}
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(const QString& text)
{
	if (file != nullptr)
	{
		bool ok;
		double tmp = locale().toDouble(text, &ok);

		if (ok)
		{
			file->getInfoTable()->setHighpassFrequency(tmp);
		}
		else
		{
			file->getInfoTable()->setHighpassFrequency(-1000000); // TODO: properly turn of/on
		}
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(double value)
{
	if (file != nullptr)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
		{
			highpassComboBox->setCurrentIndex(0);
		}
		else
		{
			highpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
		}
	}
}

void SignalFileBrowserWindow::updateManagers(int value)
{
	TrackTable* tt = file->getMontageTable()->getTrackTables()->at(value);
	trackManager->setModel(tt);
	connectModel(tt, [this] () { signalViewer->update(); });

	EventTable* et = file->getMontageTable()->getEventTables()->at(value);
	eventManager->setModel(et);
	connectModel(et, [this] () { signalViewer->update(); });
}

void SignalFileBrowserWindow::horizontalZoomIn()
{
	horizontalZoom(horizontalZoomFactor);
}

void SignalFileBrowserWindow::horizontalZoomOut()
{
	horizontalZoom(1/horizontalZoomFactor);
}

void SignalFileBrowserWindow::verticalZoomIn()
{
	verticalZoom(verticalZoomFactor);
}

void SignalFileBrowserWindow::verticalZoomOut()
{
	verticalZoom(1/verticalZoomFactor);
}

void SignalFileBrowserWindow::updateTimeMode(int mode)
{
	QAction* a = timeModeActionGroup->actions().at(mode);
	a->setChecked(true);

	timeModeStatusLabel->setText("Time Mode: " + a->text());
}

void SignalFileBrowserWindow::updatePositionStatusLabel()
{
	InfoTable* it = file->getInfoTable();

	double ratio = static_cast<double>(file->getSamplesRecorded())/it->getVirtualWidth();
	double position = (it->getPosition() + it->getPositionIndicator()*signalViewer->getCanvas()->width())*ratio;

	positionStatusLabel->setText("Position: " + file->sampleToDateTimeString(round(position)));
}

void SignalFileBrowserWindow::updateCursorStatusLabel()
{
	cursorStatusLabel->setText("Cursor at: " + file->sampleToDateTimeString(signalViewer->getCanvas()->getCursorPositionSample()));
}

void SignalFileBrowserWindow::updateMontageComboBox()
{
	if (file != nullptr)
	{
		MontageTable* mt = file->getMontageTable();
		InfoTable* it = file->getInfoTable();

		int itemCount = montageComboBox->count();
		int index = montageComboBox->currentIndex();

		for (int i = 0; i < mt->rowCount(); ++i)
		{
			montageComboBox->addItem(QString::fromStdString(mt->getName(i)));
		}

		for (int i = 0; i < itemCount; ++i)
		{
			montageComboBox->removeItem(0);
		}

		if (index >= mt->rowCount())
		{
			index = mt->rowCount() - 1;
			it->setSelectedMontage(index);
		}

		emit it->selectedMontageChanged(index);
	}
}

void SignalFileBrowserWindow::updateEventTypeComboBox()
{
	if (file != nullptr)
	{
		EventTypeTable* ett = file->getEventTypeTable();
		InfoTable* it = file->getInfoTable();

		int itemCount = eventTypeComboBox->count();
		int index = eventTypeComboBox->currentIndex();

		eventTypeComboBox->addItem("<No Type>");
		for (int i = 0; i < ett->rowCount(); ++i)
		{
			eventTypeComboBox->addItem(QString::fromStdString(ett->getName(i)));
		}

		for (int i = 0; i < itemCount; ++i)
		{
			eventTypeComboBox->removeItem(0);
		}

		if (index > ett->rowCount())
		{
			index = ett->rowCount();
			it->setSelectedType(index);
		}

		emit it->selectedTypeChanged(index);
	}
}

void SignalFileBrowserWindow::runSpikedet()
{
	if (file != nullptr)
	{
		// Build montage from code. (This is a code duplicity taken from SignalProcessor.)
		vector<AlenkaSignal::Montage<float>*> montage;

		auto code = file->getMontageTable()->getTrackTables()->at(file->getInfoTable()->getSelectedMontage())->getCode();

		for (auto e : code)
			montage.push_back(new AlenkaSignal::Montage<float>(e, globalContext.get())); // TODO: add header source

		// Run Spikedet.
		spikedetAnalysis->runAnalysis(file, montage);

		// Process the output structure.
		EventTypeTable* ett = file->getMontageTable()->getEventTypeTable();
		int index = ett->rowCount();
		ett->insertRowsBack(3);

		ett->setName("Spikedet K1", index);
		ett->setColor(QColor(0, 0, 255), index);

		ett->setName("Spikedet K2", index + 1);
		ett->setColor(QColor(0, 255, 0), index + 1);

		ett->setName("Spikedet K3", index + 2);
		ett->setColor(QColor(0, 255, 255), index + 2);

		EventTable* et = file->getMontageTable()->getEventTables()->at(file->getInfoTable()->getSelectedMontage());
		AlenkaSignal::CDetectorOutput* out = spikedetAnalysis->getOutput();
		assert(out != nullptr);

		unsigned int count = out->m_pos.size();
		if (count > 0)
		{
			assert(out->m_chan.size() == count);

			int etIndex = et->rowCount();
			et->insertRowsBack(count);

			for (unsigned int i = 0; i < count; i++)
			{
				et->setLabel("Spike " + to_string(i), etIndex + i);
				et->setType(index, etIndex + i);
				et->setPosition(out->m_pos[i]*file->getSamplingFrequency(), etIndex + i);
				et->setDuration(file->getSamplingFrequency()/20, etIndex + i);
				//et->setDuration(out->m_dur[i]*file->getSamplingFrequency(), etIndex + i);
				et->setChannel(out->m_chan[i] - 1, etIndex + i);
			}
		}
	}
}
