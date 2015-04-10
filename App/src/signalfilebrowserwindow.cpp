#include "signalfilebrowserwindow.h"

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
#include <QSettings>

using namespace std;

namespace
{
QString settingsParameters[2] = {"Martin Bárta", "SignalFileBrowserWindow"};

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

	QDockWidget* dockWidget1 = new QDockWidget("Track Manager", this);
	dockWidget1->setObjectName("Track Manager QDockWidget");
	trackManager = new TrackManager(this);
	dockWidget1->setWidget(trackManager);

	QDockWidget* dockWidget2 = new QDockWidget("Event Manager", this);
	dockWidget2->setObjectName("Event Manager QDockWidget");
	eventManager = new EventManager(this);
	dockWidget2->setWidget(eventManager);

	QDockWidget* dockWidget3 = new QDockWidget("EventType Manager", this);
	dockWidget3->setObjectName("EventType Manager QDockWidget");
	eventTypeManager = new EventTypeManager(this);
	dockWidget3->setWidget(eventTypeManager);

	QDockWidget* dockWidget4 = new QDockWidget("Montage Manager", this);
	dockWidget4->setObjectName("Montage Manager QDockWidget");
	montageManager = new MontageManager(this);
	dockWidget4->setWidget(montageManager);

	addDockWidget(Qt::RightDockWidgetArea, dockWidget1);
	tabifyDockWidget(dockWidget1, dockWidget2);
	tabifyDockWidget(dockWidget2, dockWidget3);
	tabifyDockWidget(dockWidget3, dockWidget4);

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

	// Construct File menu.
	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);
	fileMenu->addAction(saveFileAction);

	// Construct Zoom menu.
	QMenu* zoomMenu = menuBar()->addMenu("&Zoom");

	zoomMenu->addAction(horizontalZoomInAction);
	zoomMenu->addAction(horizontalZoomOutAction);
	zoomMenu->addAction(verticalZoomInAction);
	zoomMenu->addAction(verticalZoomOutAction);

	// Toolbars.
	const int spacing = 3;

	// Construct File toolbar.
	QToolBar* fileToolBar = addToolBar("File");
	fileToolBar->setObjectName("File QToolBar");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

	// Construct Filter toolbar.
	QToolBar* filterToolBar = addToolBar("Filter");
	filterToolBar->setObjectName("Filter QToolBar");
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

	// Construct Montage toolbar.
	QToolBar* montageToolBar = addToolBar("Montage");
	montageToolBar->setObjectName("Montage QToolBar");
	montageToolBar->layout()->setSpacing(spacing);

	montageToolBar->addWidget(new QLabel("Montage:", this));
	montageComboBox = new QComboBox(this);
	montageToolBar->addWidget(montageComboBox);

	// Construct Zoom toolbar.
	QToolBar* zoomToolBar = addToolBar("Zoom");
	zoomToolBar->setObjectName("Zoom QToolBar");
	zoomToolBar->layout()->setSpacing(spacing);

	zoomToolBar->addAction(horizontalZoomInAction);
	zoomToolBar->addAction(horizontalZoomOutAction);
	zoomToolBar->addAction(verticalZoomInAction);
	zoomToolBar->addAction(verticalZoomOutAction);

	// Restore settings.
	QSettings settings(settingsParameters[0], settingsParameters[1]);
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
}

SignalFileBrowserWindow::~SignalFileBrowserWindow()
{
	delete file;
}

void SignalFileBrowserWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings(settingsParameters[0], settingsParameters[1]);
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());

	QMainWindow::closeEvent(event);
}

void SignalFileBrowserWindow::connectModelToUpdate(QAbstractTableModel* model)
{
	connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), signalViewer, SLOT(update()));
	connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), signalViewer, SLOT(update()));
	connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)), signalViewer, SLOT(update()));
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

		emit tt->dataChanged(tt->index(0, static_cast<int>(TrackTable::Collumn::amplitude)), tt->index(tt->rowCount() - 1, static_cast<int>(TrackTable::Collumn::amplitude)));
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

		// Pass the file to the widget responsible for rendering.
		signalViewer->changeFile(file);

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

		highpassComboBox->clear();
		highpassComboBox->addItems(comboOptions);
		connect(highpassComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(highpassComboBoxUpdate(QString)));
		connect(it, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(highpassComboBoxUpdate(double)));

		connect(notchCheckBox, SIGNAL(toggled(bool)), it, SLOT(setNotch(bool)));
		connect(it, SIGNAL(notchChanged(bool)), notchCheckBox, SLOT(setChecked(bool)));

		// Update the Montage toolbar.
		montageComboBox->setModel(file->getMontageTable());
		connect(montageComboBox, SIGNAL(currentIndexChanged(int)), it, SLOT(setSelectedMontage(int)));
		connect(it, SIGNAL(selectedMontageChanged(int)), montageComboBox, SLOT(setCurrentIndex(int)));

		// Update the managers.
		eventTypeManager->setModel(file->getEventTypeTable());
		montageManager->setModel(file->getMontageTable());

		connect(it, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateManagers(int)));

		// Connect slot SignalViewer::update() to make sure that the SignalViewer gets updated when needed.
		connect(it, SIGNAL(virtualWidthChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(positionChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(lowpassFrequencyChanged(double)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(highpassFrequencyChanged(double)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(notchChanged(bool)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(selectedMontageChanged(int)), signalViewer, SLOT(update()));
		connect(it, SIGNAL(sectedTrackChanged(int)), signalViewer, SLOT(update()));

		connectModelToUpdate(file->getMontageTable());
		connectModelToUpdate(file->getEventTypeTable());

		// Emit all signals to ensure there are no uninitialized controls.
		it->emitAllSignals();
	}
}

void SignalFileBrowserWindow::closeFile()
{
	delete file;
	file = nullptr;
	signalViewer->changeFile(nullptr);

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

void SignalFileBrowserWindow::updateManagers(int value)
{
	TrackTable* tt = file->getMontageTable()->getTrackTables()->at(value);
	trackManager->setModel(tt);
	connectModelToUpdate(tt);

	EventTable* et = file->getMontageTable()->getEventTables()->at(value);
	eventManager->setModel(et);
	connectModelToUpdate(et);
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
