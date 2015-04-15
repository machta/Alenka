#include "signalfilebrowserwindow.h"

#include "options.h"
#include "signalviewer.h"
#include "DataFile/gdf2.h"
#include "trackmanager.h"
#include "eventmanager.h"
#include "eventtypemanager.h"
#include "montagemanager.h"

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

using namespace std;

namespace
{
const double horizontalZoomFactor = 1.3;
const double verticalZoomFactor = 1.3;
}

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle("ZSBS: Signal File Browser");

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
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	QAction* closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcut(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	QAction* saveFileAction = new QAction("Save File", this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveFileAction->setToolTip("Save the currently opened file.");
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	// Construct Zoom actions.
	QAction* horizontalZoomInAction = new QAction("Horizontal Zoom In", this);
	horizontalZoomInAction->setShortcut(QKeySequence("Alt++"));
	horizontalZoomInAction->setToolTip("Zoom in timeline.");
	connect(horizontalZoomInAction, SIGNAL(triggered()), this, SLOT(horizontalZoomIn()));

	QAction* horizontalZoomOutAction = new QAction("Horizontal Zoom Out", this);
	horizontalZoomOutAction->setShortcut(QKeySequence("Alt+-"));
	horizontalZoomOutAction->setToolTip("Zoom out timeline.");
	connect(horizontalZoomOutAction, SIGNAL(triggered()), this, SLOT(horizontalZoomOut()));

	QAction* verticalZoomInAction = new QAction("Vertical Zoom In", this);
	verticalZoomInAction->setShortcut(QKeySequence("Shift++"));
	verticalZoomInAction->setToolTip("Zoom in amplitudes of signals.");
	connect(verticalZoomInAction, SIGNAL(triggered()), this, SLOT(verticalZoomIn()));

	QAction* verticalZoomOutAction = new QAction("Vertical Zoom Out", this);
	verticalZoomOutAction->setShortcut(QKeySequence("Shift+-"));
	verticalZoomOutAction->setToolTip("Zoom out amplitudes of signals.");
	connect(verticalZoomOutAction, SIGNAL(triggered()), this, SLOT(verticalZoomOut()));

	// Construct Time Mode action group.
	timeModeActionGroup = new QActionGroup(this);

	QAction* timeModeAction0 = new QAction("Sample", this);
	timeModeAction0->setToolTip("Samples from the start.");
	connect(timeModeAction0, SIGNAL(triggered()), this, SLOT(timeMode0()));
	timeModeAction0->setActionGroup(timeModeActionGroup);
	timeModeAction0->setCheckable(true);

	QAction* timeModeAction1 = new QAction("Offset", this);
	timeModeAction1->setToolTip("Time offset from the start.");
	connect(timeModeAction1, SIGNAL(triggered()), this, SLOT(timeMode1()));
	timeModeAction1->setActionGroup(timeModeActionGroup);
	timeModeAction1->setCheckable(true);

	QAction* timeModeAction2 = new QAction("Real", this);
	timeModeAction2->setToolTip("Real time.");
	connect(timeModeAction2, SIGNAL(triggered()), this, SLOT(timeMode2()));
	timeModeAction2->setActionGroup(timeModeActionGroup);
	timeModeAction2->setCheckable(true);

	// Toolbars.
	const int spacing = 3;

	// Construct File toolbar.
	QToolBar* fileToolBar = addToolBar("File Toolbar");
	fileToolBar->setObjectName("File QToolBar");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

	// Construct Filter toolbar.
	QToolBar* filterToolBar = addToolBar("Filter Toolbar");
	filterToolBar->setObjectName("Filter QToolBar");
	filterToolBar->layout()->setSpacing(spacing);

	filterToolBar->addWidget(new QLabel("LF:", this));
	lowpassComboBox = new QComboBox(this);
	lowpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	lowpassComboBox->setMaximumWidth(150);
	lowpassComboBox->setEditable(true);
	filterToolBar->addWidget(lowpassComboBox);

	filterToolBar->addWidget(new QLabel("HF:", this));
	highpassComboBox = new QComboBox(this);
	highpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	highpassComboBox->setMaximumWidth(150);
	highpassComboBox->setEditable(true);
	filterToolBar->addWidget(highpassComboBox);

	notchCheckBox = new QCheckBox("Notch:", this);
	notchCheckBox->setLayoutDirection(Qt::RightToLeft);
	filterToolBar->addWidget(notchCheckBox);

	// Construct Select toolbar.
	QToolBar* selectToolBar = addToolBar("Select Toolbar");
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

	// Construct Zoom toolbar.
	QToolBar* zoomToolBar = addToolBar("Zoom Toolbar");
	zoomToolBar->setObjectName("Zoom QToolBar");
	zoomToolBar->layout()->setSpacing(spacing);

	zoomToolBar->addAction(horizontalZoomInAction);
	zoomToolBar->addAction(horizontalZoomOutAction);
	zoomToolBar->addAction(verticalZoomInAction);
	zoomToolBar->addAction(verticalZoomOutAction);

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

	// Construct status bar.
	timeStatusLabel = new QLabel(this);
	positionStatusLabel = new QLabel(this);
	cursorStatusLabel = new QLabel(this);

	statusBar()->addPermanentWidget(timeStatusLabel);
	statusBar()->addPermanentWidget(positionStatusLabel);
	statusBar()->addPermanentWidget(cursorStatusLabel);

	// Restore settings.
	restoreGeometry(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry").toByteArray());
	restoreState(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state").toByteArray());
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
			tt->setAmplitude(tt->getAmplitude(i)*factor, i);
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
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "GDF file (*.gdf)");

	if (fileName.isNull() == false)
	{
		// Open the file.
		delete file;

		QFileInfo fi(fileName); //TODO: perhaps add additional error checking (exists(), canbeopen(), ...)
		file = new GDF2((fi.path() + QDir::separator() + fi.completeBaseName()).toStdString());

		InfoTable* it = file->getInfoTable();

		// Check for any values in InfoTable that could make trouble.
		if (it->getSelectedMontage() < 0 || it->getSelectedMontage() >= file->getMontageTable()->rowCount())
		{
			it->setSelectedMontage(0);
		}

		// Pass the file to the child widgets.
		signalViewer->changeFile(file);		
		eventManager->changeFile(file);

		// Update Filter toolbar.
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

		// Update the Select toolbar.
		connectModel(file->getMontageTable(), [this] () { updateMontageComboBox(); });
		updateMontageComboBox();//*
		connect(montageComboBox, SIGNAL(currentIndexChanged(int)), it, SLOT(setSelectedMontage(int)));
		connect(it, SIGNAL(selectedMontageChanged(int)), montageComboBox, SLOT(setCurrentIndex(int)));

		connectModel(file->getEventTypeTable(), [this] () { updateEventTypeComboBox(); });
		updateEventTypeComboBox();//*
		connect(eventTypeComboBox, SIGNAL(currentIndexChanged(int)), it, SLOT(setSelectedType(int)));
		connect(it, SIGNAL(selectedTypeChanged(int)), eventTypeComboBox, SLOT(setCurrentIndex(int)));

		// Update the managers.
		eventTypeManager->setModel(file->getEventTypeTable());
		montageManager->setModel(file->getMontageTable());

		connect(it, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateManagers(int)));

		// Update the status bar.
		timeStatusLabel->setText("Start: " + file->sampleToDateTimeString(0, InfoTable::TimeMode::real) + " Total time: " + file->sampleToDateTimeString(file->getSamplesRecorded(), InfoTable::TimeMode::offset));

		connect(it, SIGNAL(positionChanged(int)), this, SLOT(updatePositionStatusLabel()));
		connect(signalViewer->getCanvas(), SIGNAL(cursorPositionSampleChanged(int)), this, SLOT(updateCursorStatusLabel()));

		// Connect slot SignalViewer::update() to make sure that the SignalViewer gets updated when needed.
		connect(it, SIGNAL(virtualWidthChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(positionChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(lowpassFrequencyChanged(double)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(highpassFrequencyChanged(double)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(notchChanged(bool)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(selectedMontageChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(timeModeChanged(int)), this, SLOT(updateTimeMode(int)));

		connectModel(file->getMontageTable(), [this] () { signalViewer->update(); });
		connectModel(file->getEventTypeTable(), [this] () { signalViewer->update(); });

		// Emit all signals to ensure there are no uninitialized controls.
		it->emitAllSignals();
	}
}

void SignalFileBrowserWindow::closeFile()
{
	delete file;
	file = nullptr;

	signalViewer->changeFile(nullptr);
	eventManager->changeFile(nullptr);

	signalViewer->update();
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
		double tmp = locale().toDouble(text, &ok);
		if (ok)
		{
			file->getInfoTable()->setLowpassFrequency(tmp);
		}
		else
		{
			lowpassComboBox->setCurrentIndex(0);
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
			lowpassComboBox->setCurrentIndex(0);
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
	timeModeActionGroup->actions().at(mode)->setChecked(true);
}

void SignalFileBrowserWindow::updatePositionStatusLabel()
{
	double ratio = file->getSamplesRecorded()/file->getInfoTable()->getVirtualWidth();
	int position = file->getInfoTable()->getPosition()*ratio;

	positionStatusLabel->setText("Position: " + file->sampleToDateTimeString(position));
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
