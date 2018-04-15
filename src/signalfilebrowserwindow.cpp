#include "signalfilebrowserwindow.h"

#include "../Alenka-File/include/AlenkaFile/edf.h"
#include "../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../Alenka-Signal/include/AlenkaSignal/openclcontext.h"
#include "DataModel/opendatafile.h"
#include "DataModel/undocommandfactory.h"
#include "DataModel/vitnessdatamodel.h"
#include "Manager/eventmanager.h"
#include "Manager/eventtablemodel.h"
#include "Manager/eventtypemanager.h"
#include "Manager/eventtypetablemodel.h"
#include "Manager/filtermanager.h"
#include "Manager/montagemanager.h"
#include "Manager/montagetablemodel.h"
#include "Manager/trackmanager.h"
#include "Manager/tracktablemodel.h"
#include "Manager/videoplayer.h"
#include "SignalProcessor/automaticmontage.h"
#include "SignalProcessor/bipolarmontage.h"
#include "SignalProcessor/clusteranalysis.h"
#include "SignalProcessor/defaultmontage.h"
#include "SignalProcessor/modifiedspikedetanalysis.h"
#include "SignalProcessor/signalprocessor.h"
#include "SignalProcessor/spikedetanalysis.h"
#include "Sync/syncdialog.h"
#include "canvas.h"
#include "error.h"
#include "filetype.h"
#include "montagetemplatedialog.h"
#include "myapplication.h"
#include "options.h"
#include "signalviewer.h"
#include "spikedetsettingsdialog.h"
#include <localeoverride.h>

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QtWidgets>

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace {

const char *TITLE = "Signal File Browser";

QString headerFilePath() {
  return MyApplication::makeAppSubdir({"montageHeader.cl"}).absolutePath();
}

void saveMontageHeader() {
  QFile headerFile(headerFilePath());
  string text = OpenDataFile::infoTable.getGlobalMontageHeader().toStdString();

  if (headerFile.open(QIODevice::WriteOnly)) {
    headerFile.write(text.c_str());
    headerFile.close();
  } else {
    cerr << "Error writing file " << headerFilePath().toStdString() << endl;
  }
}

void errorMessage(QWidget *parent, const string &text,
                  const QString &title = "Error") {
  QString qText = QString::fromStdString(text);
  // Use only the first 10 lines. This will ensure we don't get huge messages.
  qText = qText.section('\n', 0, 9);

  QString padding(max(0, title.size() * 2 - qText.size()), ' ');
  QMessageBox::critical(parent, title, qText + padding);
}

} // namespace

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget *parent)
    : QMainWindow(parent), fileResources(new OpenFileResources) {
  setWindowTitle(TITLE);

  OpenDataFile::kernelCache = make_unique<KernelCache>();

  autoSaveTimer = new QTimer(this);

  undoStack = new QUndoStack(this);
  connect(undoStack, SIGNAL(cleanChanged(bool)), this,
          SLOT(cleanChanged(bool)));

  view = new QQuickWidget(this);
  view->setResizeMode(QQuickWidget::SizeRootObjectToView);
  setFilePathInQML(); // define filePath
  view->setSource(QUrl(QStringLiteral("qrc:/main.qml")));

  QQuickItem *root = view->rootObject();
  connect(root, SIGNAL(switchToAlenka()), this, SLOT(switchToAlenka()));
  connect(root, SIGNAL(exit()), this, SLOT(close()));
  connect(root, SIGNAL(exportDialog()), this, SLOT(exportDialog()));
  connect(root, SIGNAL(saveSession(QString)), &OpenDataFile::infoTable,
          SLOT(setElkoSession(QString)));

  signalViewer = new SignalViewer(this);

  stackedWidget = new QStackedWidget;
  stackedWidget->addWidget(view);
  stackedWidget->addWidget(signalViewer);
  stackedWidget->setCurrentIndex(1);

  setCentralWidget(stackedWidget);

  openDataFile = make_unique<OpenDataFile>();

  // Set up Signal analysis.
  spikedetAnalysis = new SpikedetAnalysis();
  signalAnalysis.push_back(unique_ptr<Analysis>(spikedetAnalysis));

  modifiedSpikedetAnalysis = new ModifiedSpikedetAnalysis();
  signalAnalysis.push_back(unique_ptr<Analysis>(modifiedSpikedetAnalysis));

  auto settings = spikedetAnalysis->getSettings();
  SpikedetSettingsDialog::resetSettings(&settings, &spikeDuration);
  spikedetAnalysis->setSettings(settings);
  modifiedSpikedetAnalysis->setSettings(settings);

  clusterAnalysis = new ClusterAnalysis();
  signalAnalysis.push_back(unique_ptr<Analysis>(clusterAnalysis));

  centeringClusterAnalysis = new CenteringClusterAnalysis();
  signalAnalysis.push_back(unique_ptr<Analysis>(centeringClusterAnalysis));

  // Construct dock widgets.
  setDockNestingEnabled(true);

  auto trackManagerDockWidget = new QDockWidget("Track Manager", this);
  trackManagerDockWidget->setObjectName("Track Manager QDockWidget");
  trackManager = new TrackManager(this);
  trackManagerDockWidget->setWidget(trackManager);

  auto eventManagerDockWidget = new QDockWidget("Event Manager", this);
  eventManagerDockWidget->setObjectName("Event Manager QDockWidget");
  eventManager = new EventManager(this);
  eventManager->setReferences(signalViewer->getCanvas());
  eventManagerDockWidget->setWidget(eventManager);

  auto eventTypeManagerDockWidget = new QDockWidget("EventType Manager", this);
  eventTypeManagerDockWidget->setObjectName("EventType Manager QDockWidget");
  eventTypeManager = new EventTypeManager(this);
  eventTypeManagerDockWidget->setWidget(eventTypeManager);

  auto montageManagerDockWidget = new QDockWidget("Montage Manager", this);
  montageManagerDockWidget->setObjectName("Montage Manager QDockWidget");
  montageManager = new MontageManager(this);
  montageManagerDockWidget->setWidget(montageManager);

  auto filterManagerDockWidget = new QDockWidget("Filter Manager", this);
  filterManagerDockWidget->setObjectName("Filter Manager QDockWidget");
  filterManager = new FilterManager(this);
  filterManagerDockWidget->setWidget(filterManager);

  auto vieoPlayerDockWidget = new QDockWidget("Video Player", this);
  vieoPlayerDockWidget->setObjectName("Video Player QDockWidget");
  videoPlayer = new VideoPlayer(this);
  vieoPlayerDockWidget->setWidget(videoPlayer);

  addDockWidget(Qt::RightDockWidgetArea, trackManagerDockWidget);
  tabifyDockWidget(trackManagerDockWidget, eventManagerDockWidget);
  tabifyDockWidget(eventManagerDockWidget, eventTypeManagerDockWidget);
  tabifyDockWidget(eventTypeManagerDockWidget, montageManagerDockWidget);
  tabifyDockWidget(montageManagerDockWidget, filterManagerDockWidget);
  tabifyDockWidget(filterManagerDockWidget, vieoPlayerDockWidget);

  // Construct File actions.
  QAction *openFileAction = new QAction("&Open File...", this);
  openFileAction->setShortcut(QKeySequence::Open);
  openFileAction->setToolTip("Open an existing file");
  openFileAction->setStatusTip(openFileAction->toolTip());
  openFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

  closeFileAction = new QAction("Close File", this);
  closeFileAction->setShortcut(QKeySequence::Close);
  closeFileAction->setToolTip("Close the currently opened file");
  closeFileAction->setStatusTip(closeFileAction->toolTip());
  closeFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

  saveFileAction = new QAction("Save File", this);
  saveFileAction->setShortcut(QKeySequence::Save);
  saveFileAction->setToolTip("Save the currently opened file");
  saveFileAction->setStatusTip(saveFileAction->toolTip());
  saveFileAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  saveFileAction->setEnabled(false);
  connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

  exportToEdfAction = new QAction("Export current file to EDF...", this);
  exportToEdfAction->setToolTip("Export the opened file to EDF");
  exportToEdfAction->setStatusTip(exportToEdfAction->toolTip());
  connect(exportToEdfAction, SIGNAL(triggered()), this, SLOT(exportToEdf()));

  QAction *undoAction = undoStack->createUndoAction(this);
  undoAction->setShortcut(QKeySequence::Undo);
  // undoAction->setIcon(QIcon::fromTheme("edit-undo"));
  // undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));

  QAction *redoAction = undoStack->createRedoAction(this);
  redoAction->setShortcut(QKeySequence::Redo);
  // redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
  // TODO: Get proper icons for undo/redo.

  // Construct Zoom actions.
  QAction *horizontalZoomInAction = new QAction("Horizontal Zoom In", this);
  horizontalZoomInAction->setIcon(QIcon(":/icons/zoom_in_horizontal.png"));
  horizontalZoomInAction->setShortcut(QKeySequence("Alt++"));
  horizontalZoomInAction->setToolTip("Zoom in time line");
  horizontalZoomInAction->setStatusTip(horizontalZoomInAction->toolTip());
  connect(horizontalZoomInAction, &QAction::triggered,
          [this]() { signalViewer->getCanvas()->horizontalZoom(false); });

  QAction *horizontalZoomOutAction = new QAction("Horizontal Zoom Out", this);
  horizontalZoomOutAction->setIcon(QIcon(":/icons/zoom_out_horizontal.png"));
  horizontalZoomOutAction->setShortcut(QKeySequence("Alt+-"));
  horizontalZoomOutAction->setToolTip("Zoom out time line");
  horizontalZoomOutAction->setStatusTip(horizontalZoomOutAction->toolTip());
  connect(horizontalZoomOutAction, &QAction::triggered,
          [this]() { signalViewer->getCanvas()->horizontalZoom(true); });

  QAction *verticalZoomInAction = new QAction("Vertical Zoom In", this);
  verticalZoomInAction->setIcon(QIcon(":/icons/zoom_in_vertical.png"));
  verticalZoomInAction->setShortcut(QKeySequence("Shift++"));
  verticalZoomInAction->setToolTip("Zoom in amplitudes of signals");
  verticalZoomInAction->setStatusTip(verticalZoomInAction->toolTip());
  connect(verticalZoomInAction, SIGNAL(triggered(bool)), this,
          SLOT(verticalZoomIn()));
  connect(signalViewer->getCanvas(), SIGNAL(shiftZoomUp()), this,
          SLOT(verticalZoomIn()));

  QAction *verticalZoomOutAction = new QAction("Vertical Zoom Out", this);
  verticalZoomOutAction->setIcon(QIcon(":/icons/zoom_out_vertical.png"));
  verticalZoomOutAction->setShortcut(QKeySequence("Shift+-"));
  verticalZoomOutAction->setToolTip("Zoom out amplitudes of signals");
  verticalZoomOutAction->setStatusTip(verticalZoomOutAction->toolTip());
  connect(verticalZoomOutAction, SIGNAL(triggered(bool)), this,
          SLOT(verticalZoomOut()));
  connect(signalViewer->getCanvas(), SIGNAL(shiftZoomDown()), this,
          SLOT(verticalZoomOut()));

  // Construct Keyboard actions.
  QAction *shiftAction = new QAction("Shift", this);
  shiftAction->setShortcut(QKeySequence("Shift"));
  shiftAction->setCheckable(true);
  shiftAction->setToolTip(
      "Simulate pressed down shift button when using a touch screen");
  shiftAction->setStatusTip(shiftAction->toolTip());

  QAction *ctrlAction = new QAction("Ctrl", this);
  ctrlAction->setShortcut(QKeySequence("Ctrl"));
  ctrlAction->setCheckable(true);
  ctrlAction->setToolTip(
      "Simulate pressed down ctrl button when using a touch screen");
  ctrlAction->setStatusTip(ctrlAction->toolTip());

  connect(shiftAction, &QAction::toggled, [this, shiftAction, ctrlAction]() {
    signalViewer->getCanvas()->shiftButtonCheckEvent(shiftAction->isChecked());
    if (shiftAction->isChecked())
      ctrlAction->setChecked(false);
  });

  connect(ctrlAction, &QAction::toggled, [this, ctrlAction, shiftAction]() {
    signalViewer->getCanvas()->ctrlButtonCheckEvent(ctrlAction->isChecked());
    if (ctrlAction->isChecked())
      shiftAction->setChecked(false);
  });

  // Construct Spikedet actions.
  for (unsigned int i = 0; i < signalAnalysis.size(); ++i) {
    QString name = "Run " + QString::fromStdString(signalAnalysis[i]->name());

    QAction *action = new QAction(name, this);
    action->setToolTip(name + " on the current montage");
    action->setStatusTip(action->toolTip());
    connect(action, &QAction::triggered, [this, i]() { runSignalAnalysis(i); });

    analysisActions.push_back(action);
  }

  QAction *runSpikedetAction = analysisActions[0];
  runSpikedetAction->setIcon(QIcon(":/icons/play.png"));

  QAction *spikedetSettingsAction =
      new QAction(QIcon(":/icons/settings.png"), "Spikedet Settings...", this);
  spikedetSettingsAction->setToolTip("Change Spikedet settings");
  spikedetSettingsAction->setStatusTip(spikedetSettingsAction->toolTip());
  connect(spikedetSettingsAction, &QAction::triggered, [this]() {
    DETECTOR_SETTINGS settings = spikedetAnalysis->getSettings();
    double newDuration = spikeDuration;

    SpikedetSettingsDialog dialog(&settings, &newDuration, this);

    if (dialog.exec() == QDialog::Accepted) {
      spikedetAnalysis->setSettings(settings);
      modifiedSpikedetAnalysis->setSettings(settings);
      spikeDuration = newDuration;
    }
  });

  // Construct Time Mode action group.
  timeModeActionGroup = new QActionGroup(this);

  QAction *timeModeAction0 = new QAction("Sample", this);
  timeModeAction0->setToolTip("Samples from the start");
  timeModeAction0->setStatusTip(timeModeAction0->toolTip());
  timeModeAction0->setActionGroup(timeModeActionGroup);
  timeModeAction0->setCheckable(true);
  connect(timeModeAction0, &QAction::triggered, [this]() { mode(0); });

  QAction *timeModeAction1 = new QAction("Offset", this);
  timeModeAction1->setToolTip("Time offset from the start");
  timeModeAction1->setStatusTip(timeModeAction1->toolTip());
  timeModeAction1->setActionGroup(timeModeActionGroup);
  timeModeAction1->setCheckable(true);
  connect(timeModeAction1, &QAction::triggered, [this]() { mode(1); });

  QAction *timeModeAction2 = new QAction("Real", this);
  timeModeAction2->setToolTip("Real time and date");
  timeModeAction2->setStatusTip(timeModeAction2->toolTip());
  timeModeAction2->setActionGroup(timeModeActionGroup);
  timeModeAction2->setCheckable(true);
  connect(timeModeAction2, &QAction::triggered, [this]() { mode(2); });

  // Construct Time Line Interval action group.
  timeLineIntervalActionGroup = new QActionGroup(this);

  QAction *timeLineOffAction = new QAction("Off", this);
  timeLineOffAction->setToolTip("Turn off the time lines");
  timeLineOffAction->setStatusTip(timeLineOffAction->toolTip());
  connect(timeLineOffAction, &QAction::triggered, [this]() {
    if (fileResources->file)
      OpenDataFile::infoTable.setTimeLineInterval(0);
  });

  setTimeLineIntervalAction = new QAction("Set...", this);
  setTimeLineIntervalAction->setActionGroup(timeLineIntervalActionGroup);
  connect(setTimeLineIntervalAction, &QAction::triggered, [this]() {
    if (fileResources->file) {
      double value = OpenDataFile::infoTable.getTimeLineInterval();
      if (value == 0)
        value = 1;

      bool ok;
      value = QInputDialog::getDouble(
          this, "Set the interval",
          "Please, enter the value for the time line interval here:", value, 0,
          1000 * 1000 * 1000, 2, &ok);

      if (ok)
        OpenDataFile::infoTable.setTimeLineInterval(value);
    }
  });

  // Construct SyncDialog.
  syncDialog = new SyncDialog(this);

  QAction *showSyncDialog = new QAction("Show Sync Dialog...", this);
  connect(showSyncDialog, SIGNAL(triggered(bool)), syncDialog, SLOT(show()));

  synchronize = new QAction("Synchronize", this);
  synchronize->setCheckable(true);
  synchronize->setChecked(true);
  connect(synchronize, SIGNAL(triggered(bool)), syncDialog,
          SLOT(setShouldSynchronize(bool)));

  // Tool bars.
  const int spacing = 3;

  // Construct File tool bar.
  QToolBar *fileToolBar = addToolBar("File Tool Bar");
  fileToolBar->setObjectName("File QToolBar");
  fileToolBar->layout()->setSpacing(spacing);

  fileToolBar->addAction(openFileAction);
  fileToolBar->addAction(closeFileAction);
  fileToolBar->addAction(saveFileAction);
  addAction(undoAction); // Add these to this widget to keep the shortcuts
                         // working, but do't include it in the toolbar.
  addAction(redoAction);

  // Construct Filter tool bar.
  QToolBar *filterToolBar = addToolBar("Filter Tool Bar");
  filterToolBar->setObjectName("Filter QToolBar");
  filterToolBar->layout()->setSpacing(spacing);

  QLabel *label = new QLabel("HF:", this);
  label->setToolTip("High-pass Filter frequency");
  filterToolBar->addWidget(label);
  highpassComboBox = new QComboBox(this);
  highpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  highpassComboBox->setMaximumWidth(150);
  highpassComboBox->setEditable(true);
  highpassComboBox->setValidator(new QDoubleValidator);
  filterToolBar->addWidget(highpassComboBox);

  label = new QLabel("LF:", this);
  label->setToolTip("Low-pass Filter frequency");
  filterToolBar->addWidget(label);
  lowpassComboBox = new QComboBox(this);
  lowpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  lowpassComboBox->setMaximumWidth(150);
  lowpassComboBox->setEditable(true);
  lowpassComboBox->setValidator(new QDoubleValidator);
  filterToolBar->addWidget(lowpassComboBox);

  notchCheckBox = new QCheckBox("Notch:", this);
  notchCheckBox->setToolTip("Notch Filter on/off");
  notchCheckBox->setLayoutDirection(Qt::RightToLeft);
  filterToolBar->addWidget(notchCheckBox);

  // Construct Select tool bar.
  QToolBar *selectToolBar = addToolBar("Select Tool bar");
  selectToolBar->setObjectName("Select QToolBar");
  selectToolBar->layout()->setSpacing(spacing);

  auto addMontageButton = new QPushButton("Montage:");
  addMontageButton->setToolTip("Add new montage");
  addMontageButton->setStatusTip(addMontageButton->toolTip());
  selectToolBar->addWidget(addMontageButton);

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

  selectToolBar->addSeparator();
  label = new QLabel("Res:", this);
  label->setToolTip("Vertical resolution in volts per centimeter");
  selectToolBar->addWidget(label);
  resolutionComboBox = new QComboBox(this);
  resolutionComboBox->setEditable(true);
  resolutionComboBox->setValidator(new QDoubleValidator);
  selectToolBar->addWidget(resolutionComboBox);

  unitsComboBox = new QComboBox(this);
  unitsComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  unitsComboBox->addItems(QStringList() << QChar(0x00B5) + QString("V") << "mV"
                                        << "V"
                                        << "kV"
                                        << "MV");
  selectToolBar->addWidget(unitsComboBox);
  connect(unitsComboBox, SIGNAL(currentIndexChanged(int)),
          &OpenDataFile::infoTable, SLOT(setSampleUnits(int)));
  connect(&OpenDataFile::infoTable, SIGNAL(sampleUnitsChanged(int)),
          unitsComboBox, SLOT(setCurrentIndex(int)));
  selectToolBar->addWidget(new QLabel("/cm"));

  // Construct Zoom tool bar.
  QToolBar *zoomToolBar = addToolBar("Zoom Tool Bar");
  zoomToolBar->setObjectName("Zoom QToolBar");
  int spacingMulti = 1;
  string mode;
  programOption("mode", mode);
  bool tabletMode = mode == "tablet" || mode == "tablet-full";
  if (tabletMode) {
    zoomToolBar->setMinimumHeight(40);
    zoomToolBar->setIconSize(QSize(40, 40));
    spacingMulti = 3;
  }
  zoomToolBar->layout()->setSpacing(spacing * spacingMulti);

  zoomToolBar->addAction(horizontalZoomInAction);
  zoomToolBar->addAction(horizontalZoomOutAction);
  zoomToolBar->addAction(verticalZoomInAction);
  zoomToolBar->addAction(verticalZoomOutAction);

  // Construct Keyboard tool bar.
  QToolBar *keyboardToolBar = addToolBar("Keyboard Tool Bar");
  keyboardToolBar->setObjectName("Keyboard QToolBar");
  keyboardToolBar->layout()->setSpacing(spacing * spacingMulti);

  keyboardToolBar->addAction(shiftAction);
  keyboardToolBar->addAction(ctrlAction);

  // Construct Spikedet tool bar.
  QToolBar *spikedetToolBar = addToolBar("Spikedet Tool Bar");
  spikedetToolBar->setObjectName("Spikedet QToolBar");
  spikedetToolBar->layout()->setSpacing(spacing * spacingMulti);

  spikedetToolBar->addAction(runSpikedetAction);
  spikedetToolBar->addAction(spikedetSettingsAction);

  // Construct Switch apps tool bar.
  QToolBar *switchToolBar = addToolBar("Switch Tool Bar");
  switchToolBar->setObjectName("Switch QToolBar");
  switchToolBar->layout()->setSpacing(spacing * 3);

  // Construct Switch button.
  switchButton = new QPushButton("Switch to Elko", this);
  if (tabletMode)
    switchButton->setMinimumSize(QSize(150, 40));
  switchButton->setToolTip("Switch between Alenka and Elko");
  switchButton->setStatusTip(switchButton->toolTip());
  switchButton->setEnabled(false);
  switchToolBar->addWidget(switchButton);

  connect(switchButton, &QPushButton::pressed, [this, mode]() {
    if (stackedWidget->currentIndex() == 1) {
      logToFile("Switching to Elko.");
      signalViewer->getCanvas()->setPaintingDisabled(false);

      setFilePathInQML();
      stackedWidget->setCurrentIndex(0);

      // Remember the state so that we can put everything where it was before
      // the switch.
      windowState = saveState();
      // This is to prevent saving of full-screen geometry on exit from Elko.
      windowGeometry = saveGeometry();

      for (auto e : findChildren<QToolBar *>())
        e->hide();

      for (auto e : findChildren<QDockWidget *>())
        e->hide();

      menuBar()->hide();
      statusBar()->hide();

      if (mode == "tablet-full")
        showFullScreen();
    }
  });

  // Construct File menu.
  fileMenu = menuBar()->addMenu("&File");

  fileMenu->addAction(openFileAction);
  fileMenu->addAction(closeFileAction);
  fileMenu->addAction(saveFileAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exportToEdfAction);
  fileMenu->addSeparator();
  fileMenu->addAction(undoAction);
  fileMenu->addAction(redoAction);
  fileMenu->addSeparator();
  addRecentFilesActions();

  // Construct View menu.
  QMenu *viewMenu = menuBar()->addMenu("&View");
  // TODO: Add shortcuts with & to all menu options.

  QAction *screenshotAction =
      new QAction("Save Signal View screenshot...", this);
  connect(screenshotAction, &QAction::triggered, [this] {
    auto canvas = signalViewer->getCanvas();
    QRect rectangle(QPoint(), canvas->size());

    QPixmap pixmap(rectangle.size());
    canvas->render(&pixmap, QPoint(), QRegion(rectangle));

    QString fileName = imageFilePathDialog();
    if (!fileName.isNull())
      pixmap.save(fileName);
  });
  viewMenu->addAction(screenshotAction);
  viewMenu->addSeparator();

  viewMenu->addAction(horizontalZoomInAction);
  viewMenu->addAction(horizontalZoomOutAction);
  viewMenu->addAction(verticalZoomInAction);
  viewMenu->addAction(verticalZoomOutAction);
  viewMenu->addSeparator();

  QMenu *timeModeMenu = new QMenu("Time Mode", this);
  timeModeMenu->addAction(timeModeAction0);
  timeModeMenu->addAction(timeModeAction1);
  timeModeMenu->addAction(timeModeAction2);
  viewMenu->addMenu(timeModeMenu);

  QMenu *timeLineIntervalMenu = new QMenu("Time Line Interval", this);
  timeLineIntervalMenu->addAction(timeLineOffAction);
  timeLineIntervalMenu->addAction(setTimeLineIntervalAction);
  viewMenu->addMenu(timeLineIntervalMenu);
  viewMenu->addSeparator();

  QAction *secondsPerPageAction = new QAction("Set seconds per page...", this);
  connect(secondsPerPageAction, &QAction::triggered, [this] {
    bool ok;
    double d = QInputDialog::getDouble(this, "Seconds per page",
                                       "Seconds per page:", 10, 0, 1000 * 1000,
                                       2, &ok);
    if (ok)
      setSecondsPerPage(d);
  });
  viewMenu->addAction(secondsPerPageAction);

  QAction *tenSecondsPerPageAction = new QAction("10 seconds per page", this);
  connect(tenSecondsPerPageAction, &QAction::triggered,
          [this] { setSecondsPerPage(10); });
  viewMenu->addAction(tenSecondsPerPageAction);

  // Construct Window menu.
  QMenu *windowMenu = menuBar()->addMenu("&Window");

  windowMenu->addAction(trackManagerDockWidget->toggleViewAction());
  windowMenu->addAction(eventManagerDockWidget->toggleViewAction());
  windowMenu->addAction(eventTypeManagerDockWidget->toggleViewAction());
  windowMenu->addAction(montageManagerDockWidget->toggleViewAction());
  windowMenu->addAction(filterManagerDockWidget->toggleViewAction());
  windowMenu->addAction(vieoPlayerDockWidget->toggleViewAction());

  windowMenu->addSeparator();
  windowMenu->addAction(fileToolBar->toggleViewAction());
  windowMenu->addAction(filterToolBar->toggleViewAction());
  windowMenu->addAction(selectToolBar->toggleViewAction());
  windowMenu->addAction(zoomToolBar->toggleViewAction());
  windowMenu->addAction(keyboardToolBar->toggleViewAction());
  windowMenu->addAction(spikedetToolBar->toggleViewAction());
  windowMenu->addAction(switchToolBar->toggleViewAction());

  // Construct Tools menu.
  QMenu *toolsMenu = menuBar()->addMenu("&Tools");

  QMenu *analysisMenu = toolsMenu->addMenu("Run Analysis");
  for (auto &e : analysisActions)
    analysisMenu->addAction(e);
  toolsMenu->addAction(spikedetSettingsAction);
  toolsMenu->addSeparator();

  toolsMenu->addAction(showSyncDialog);
  toolsMenu->addAction(synchronize);
  toolsMenu->addSeparator();

  QMenu *addMontageMenu = toolsMenu->addMenu("Add Montage");
  addMontageButton->setMenu(addMontageMenu);
  QAction *montageTemplatesAction = new QAction("Montage Templates...", this);
  connect(montageTemplatesAction, &QAction::triggered, [this]() {
    MontageTemplateDialog dialog(openDataFile.get());
    dialog.exec();
  });
  addMontageMenu->addAction(montageTemplatesAction);
  addMontageMenu->addSeparator();

  autoMontages.push_back(make_unique<AutomaticMontage>());
  autoMontages.push_back(make_unique<DefaultMontage>());
  autoMontages.push_back(make_unique<BipolarMontage>());
  autoMontages.push_back(make_unique<BipolarNeighboursMontage>());

  for (const auto &e : autoMontages) {
    QAction *autoMontageAction =
        new QAction("Add " + QString::fromStdString(e->getName()), this);

    connect(autoMontageAction, &QAction::triggered,
            [this, &e]() { addAutoMontage(e.get()); });

    addMontageMenu->addAction(autoMontageAction);
  }

  // Construct Help menu.
  QMenu *helpMenu = menuBar()->addMenu("&Help");

  auto userManualAction = new QAction("User Manual", this);
  connect(userManualAction, &QAction::triggered, []() {
    QDesktopServices::openUrl(
        QString("https://github.com/machta/Alenka/wiki/User-Manual"));
  });
  helpMenu->addAction(userManualAction);

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
  restoreGeometry(PROGRAM_OPTIONS->settings("SignalFileBrowserWindow geometry")
                      .toByteArray());
  restoreState(
      PROGRAM_OPTIONS->settings("SignalFileBrowserWindow state").toByteArray());

  setEnableFileActions(false);
}

SignalFileBrowserWindow::~SignalFileBrowserWindow() { closeFilePropagate(); }

QDateTime SignalFileBrowserWindow::sampleToDate(DataFile *const file,
                                                const int sample) {
  QDateTime date;
  const double daysSinceJesus = file->getStartDate();

  if (DataFile::INVALID_DATE == daysSinceJesus)
    date = QDateTime::fromSecsSinceEpoch(file->getStandardStartDate());
  else {
    double msec = daysSinceJesus - DataFile::daysUpTo1970;
    msec *= 24 * 60 * 60 * 1000;

    date = QDateTime(QDate(1970, 1, 1));
    date.setTimeSpec(Qt::UTC); // To prevent the local time-zone settings from
                               // screwing up the time.
    date = date.addMSecs(static_cast<qint64>(round(msec)));
  }

  const int timeOffset = round(sample / file->getSamplingFrequency() * 1000);
  date = date.addMSecs(timeOffset);

  return date;
}

QDateTime SignalFileBrowserWindow::sampleToOffset(DataFile *file, int sample) {
  int timeOffset = round(sample / file->getSamplingFrequency() * 1000);

  QDateTime date(QDate(1970, 1, 1));
  date = date.addMSecs(timeOffset);

  return date;
}

QString
SignalFileBrowserWindow::sampleToDateTimeString(DataFile *file, int sample,
                                                InfoTable::TimeMode mode) {
  QLocale locale;

  if (mode == InfoTable::TimeMode::size) {
    mode = OpenDataFile::infoTable.getTimeMode();
  }

  if (mode == InfoTable::TimeMode::samples) {
    return QString::number(sample);
  } else if (mode == InfoTable::TimeMode::offset) {
    QDateTime date = sampleToOffset(file, sample);
    return QString::number(date.date().day() - 1) + "d " +
           date.toString("hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
  } else if (mode == InfoTable::TimeMode::real) {
    return sampleToDate(file, sample)
        .toString("d.M.yyyy hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
  }

  return QString();
}

unique_ptr<DataFile>
SignalFileBrowserWindow::dataFileBySuffix(const QString &fileName,
                                          const vector<string> &additionalFiles,
                                          QWidget *parent) {

  const auto fileTypes = FileType::fromSuffix(fileName, additionalFiles);

  if (fileTypes.empty())
    throwDetailed(runtime_error("Unknown file extension."));

  int fileTypeIndex = 0;

  if (parent && fileTypes.size() > 1) {
    QStringList items;
    for (auto &e : fileTypes)
      items.push_back(e->name());
    fileTypeIndex = askForDataFileBackend(items, parent);
  }

  if (0 <= fileTypeIndex)
    return fileTypes[fileTypeIndex]->makeInstance();
  else
    return nullptr;
}

int SignalFileBrowserWindow::askForDataFileBackend(const QStringList &items,
                                                   QWidget *parent) {
  bool ok;
  const QString item = QInputDialog::getItem(parent, "Choose DataFile Backend",
                                             "Library:", items, 0, false, &ok);
  if (ok && !item.isEmpty())
    return items.indexOf(item);
  else
    return -1;
}

void SignalFileBrowserWindow::openCommandLineFile() {
  if (isProgramOptionSet("filename")) {
    vector<string> fn;
    programOption("filename", fn);
    vector<string> rest(fn.begin() + 1, fn.end());

    openFile(QString::fromStdString(fn[0]), rest);
  }
}

void SignalFileBrowserWindow::closeEvent(QCloseEvent *const event) {
  if (closeFile()) {
    if (windowState.isEmpty())
      windowState = saveState();

    if (windowGeometry.isEmpty())
      windowGeometry = saveGeometry();

    PROGRAM_OPTIONS->settings("SignalFileBrowserWindow state", windowState);
    PROGRAM_OPTIONS->settings("SignalFileBrowserWindow geometry",
                              windowGeometry);
    event->accept();
  } else {
    event->ignore();
  }
}

void SignalFileBrowserWindow::keyPressEvent(QKeyEvent *const event) {
  if (Qt::Key_Space == event->key()) {
    videoPlayer->togglePlay();
    event->accept();
  } else
    event->ignore();
}

vector<QMetaObject::Connection>
SignalFileBrowserWindow::connectVitness(const DataModelVitness *const vitness,
                                        std::function<void()> f) {
  vector<QMetaObject::Connection> connections;

  auto c = connect(vitness, &DataModelVitness::valueChanged, f);
  connections.push_back(c);
  c = connect(vitness, &DataModelVitness::rowsInserted, f);
  connections.push_back(c);
  c = connect(vitness, &DataModelVitness::rowsRemoved, f);
  connections.push_back(c);

  return connections;
}

void SignalFileBrowserWindow::mode(int m) {
  if (fileResources->file) {
    OpenDataFile::infoTable.setTimeMode(static_cast<InfoTable::TimeMode>(m));

    updatePositionStatusLabel();
    updateCursorStatusLabel();
  }
}

void SignalFileBrowserWindow::deleteAutoSave() {
  if (autoSaveName != "") {
    QFile(QString::fromStdString(autoSaveName)).remove();
    QFile(QString::fromStdString(autoSaveName) + "0").remove();
    QFile(QString::fromStdString(autoSaveName) + "1").remove();
  }

  autoSaveTimer->start();
}

void SignalFileBrowserWindow::setCurrentInNumericCombo(QComboBox *combo,
                                                       double value) {
  int precisionPower = static_cast<int>(pow(10, COMBO_PRECISION));
  double newValue = round(value * precisionPower);
  int count = combo->count();

  for (int i = 0; i < count; ++i) {
    bool ok;
    double itemValue = locale().toDouble(combo->itemText(i), &ok);

    if (ok && newValue == round(itemValue * precisionPower)) {
      combo->setCurrentIndex(i);

      double lastItemValue = locale().toDouble(combo->itemText(count - 1), &ok);

      if (combo->currentIndex() != count - 1 && ok &&
          newValue == round(lastItemValue * precisionPower))
        combo->removeItem(count - 1);

      return;
    }
  }
}

// TODO: Make proper sortable combo boxes instead of using this hack.
// This method is not suitable for the filter settings because the index gets
// changed multiple times, which in turn causes signal data to be read and
// processed repeatedly and needlessly. This may take considerable time, so I
// will avoid it at the cost of some "ugly" UI for now.
void SignalFileBrowserWindow::sortInLastItem(QComboBox *combo) {
  int count = combo->count();
  bool ok;
  double lastItemValue = locale().toDouble(combo->itemText(count - 1), &ok);
  assert(ok);
  int newIndex = count;

  for (int i = count - 2; 0 <= i; --i) {
    double itemValue = locale().toDouble(combo->itemText(i), &ok);

    if (ok && lastItemValue < itemValue)
      newIndex = i;
  }

  if (newIndex != count) {
    combo->removeItem(count - 1);
    combo->insertItem(newIndex,
                      locale().toString(lastItemValue, 'f', COMBO_PRECISION));
    combo->setCurrentIndex(newIndex);
  }
}

QString SignalFileBrowserWindow::imageFilePathDialog() {
  const QString filter =
      "PNG Image (*.png);;JPEG Image (*.jpg);;Bitmap Image (*.bmp)";
  QString selectedFilter;
  const QString fileName = QFileDialog::getSaveFileName(
      this, "Choose image file path", "", filter, &selectedFilter);

  if (fileName.isNull())
    return fileName;

  QFileInfo fileInfo(fileName);
  QString suffix = fileInfo.suffix();

  if (selectedFilter.contains(".png") && suffix == "png") {
    return fileName;
  } else if (selectedFilter.contains(".jpg") && suffix == "jpg") {
    return fileName;
  } else if (selectedFilter.contains(".bmp") && suffix == "bmp") {
    return fileName;
  } else {
    QMessageBox::critical(this, "Bad suffix",
                          "The file name must have either of the following "
                          "suffixes: png, jpg, or bmp.\n\nTry again.");
    return imageFilePathDialog();
  }
}

void SignalFileBrowserWindow::setSecondsPerPage(double seconds) {
  if (fileResources->file) {
    double width = signalViewer->getCanvas()->width() *
                   fileResources->file->getSamplesRecorded();
    width /= seconds * fileResources->file->getSamplingFrequency();
    OpenDataFile::infoTable.setVirtualWidth(static_cast<int>(round(width)));
  }
}

void SignalFileBrowserWindow::createDefaultMontage() {
  auto mont = make_unique<DefaultMontage>();
  addAutoMontage(mont.get());

  // The code above creates an entry in the undo stack. We don't want it so we
  // need to clear it.
  assert(1 == undoStack->count() && "There should be only one command.");
  undoStack->clear();
}

void SignalFileBrowserWindow::addRecentFilesActions() {
  for (auto e : recentFilesActions)
    fileMenu->removeAction(e);
  recentFilesActions.clear();

  QVariant recent = PROGRAM_OPTIONS->settings("resent files");

  if (!recent.isNull()) {
    QStringList list = recent.toStringList();

    if (0 < list.size()) {
      int i = 1;

      for (const auto &e : list) {
        QFileInfo fileInfo(e);

        if (fileInfo.exists()) {
          auto text = QString::fromStdString(to_string(i++) + " ");
          text = (i <= 10 ? "&" : "") + text + fileInfo.filePath();

          auto action = new QAction(text, this);
          action->setToolTip(fileInfo.filePath());
          action->setStatusTip(action->toolTip());
          fileMenu->addAction(action);
          recentFilesActions.push_back(action);

          connect(action, &QAction::triggered, [this, fileInfo]() {
            if (!closeFile())
              return;
            openFile(fileInfo.filePath());
          });
        }
      }
    }
  }
}

void SignalFileBrowserWindow::updateRecentFiles(const QFileInfo &fileInfo) {
  QString filePath = fileInfo.filePath();
  QVariant recent = PROGRAM_OPTIONS->settings("resent files");
  QStringList list, newList;

  if (!recent.isNull())
    list = recent.toStringList();

  newList.push_back(filePath);

  for (auto it = list.begin(); it != list.end(); ++it)
    if (*it != filePath)
      newList.push_back(*it);

  if (RECENT_FILE_COUNT < newList.size())
    newList.pop_back();

  PROGRAM_OPTIONS->settings("resent files", newList);
}

void SignalFileBrowserWindow::addAutoMontage(AutomaticMontage *autoMontage) {
  if (!fileResources->file)
    return;

  UndoCommandFactory *undoFactory = openDataFile->undoFactory;
  undoFactory->beginMacro("Add " +
                          QString::fromStdString(autoMontage->getName()));

  const AbstractMontageTable *mt = fileResources->dataModel->montageTable();
  int index = mt->rowCount();
  undoFactory->insertMontage(index);

  Montage m = mt->row(index);
  m.name = autoMontage->getName();
  undoFactory->changeMontage(index, m);

  autoMontage->fillTrackTable(mt->trackTable(0), mt->trackTable(index), index,
                              undoFactory);
  undoFactory->endMacro();

  assert(0 <= index && index < mt->rowCount() &&
         "Make sure the selected index is legal");
  OpenDataFile::infoTable.setSelectedMontage(index);
}

void SignalFileBrowserWindow::openFile() {
  if (!closeFile())
    return; // Close canceled -- the user chose to keep the current file open.

  QString fileName = QFileDialog::getOpenFileName(
      this, "Open File", "",
      "All files (*);;EDF files (*.edf);;GDF files (*.gdf);;MAT files (*.mat)");

  if (fileName.isNull())
    return; // No file was selected.

  openFile(fileName);
}

void SignalFileBrowserWindow::openFile(const QString &fileName,
                                       const vector<string> &additionalFiles) {
  QFileInfo fileInfo(fileName);

  if (fileInfo.exists() == false) {
    logToFileAndConsole("File '" + fileName.toStdString() + "' not found.");
    return;
  } else if (fileInfo.isReadable() == false) {
    logToFileAndConsole("File '" + fileName.toStdString() +
                        "' cannot be read.");
    return;
  } else if (fileInfo.isWritable() == false) {
    logToFileAndConsole("File '" + fileName.toStdString() +
                        "' cannot be written to.");
    return;
  }

  assert(!fileResources->file && "Make sure there is no already opened file.");

  try {
    auto filePtr = dataFileBySuffix(fileName, additionalFiles, this);
    if (!filePtr)
      return;
    fileResources->file = std::move(filePtr);
  } catch (const runtime_error &e) {
    errorMessage(this, catchDetailed(e), "Error while opening file");
    return; // Ignore opening of the file as there was an error.
  }

  logToFile("Opening file '" << fileName.toStdString() << "'.");
  setEnableFileActions(true);

  updateRecentFiles(fileInfo);
  addRecentFilesActions();

  fileResources->dataModel = UndoCommandFactory::emptyDataModel();
  auto oldDataModel = fileResources->dataModel.get();
  fileResources->file->setDataModel(oldDataModel);

  fileResources->undoFactory =
      make_unique<UndoCommandFactory>(oldDataModel, undoStack);

  openDataFile->file = fileResources->file.get();
  openDataFile->dataModel = oldDataModel;
  openDataFile->undoFactory = fileResources->undoFactory.get();

  setSecondsPerPage(10); // The default vertical zoom setting for new files.

  autoSaveName = fileResources->file->getFilePath() + ".mont.autosave";
  bool useAutoSave = false;
  if (QFileInfo(autoSaveName.c_str()).exists()) {
    auto res = QMessageBox::question(this, "Load Autosave File?",
                                     "An autosave file was "
                                     "detected. Would you like to load it?",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes);

    useAutoSave = res == QMessageBox::Yes;
  }

  LocaleOverride::executeWithCLocale([this, useAutoSave, oldDataModel]() {
    const bool secondaryFileExists = fileResources->file->load();
    if (!secondaryFileExists)
      createDefaultMontage();

    if (useAutoSave) {
      auto newDataModel = UndoCommandFactory::emptyDataModel();
      fileResources->file->setDataModel(newDataModel.get());
      const bool autosaveFileExists =
          fileResources->file->loadSecondaryFile(autoSaveName);
      assert(autosaveFileExists);
      (void)autosaveFileExists;

      fileResources->file->setDataModel(oldDataModel);
      openDataFile->undoFactory->overwriteDataModel(std::move(newDataModel),
                                                    "Restore auto-save");
    }

    DETECTOR_SETTINGS settings = AlenkaSignal::Spikedet::defaultSettings();
    OpenDataFile::infoTable.readXML(fileResources->file->getFilePath() +
                                        ".info",
                                    &settings, &spikeDuration);
  });

  cleanChanged(undoStack->isClean());
  setWindowTitle(fileInfo.fileName() + " - " + TITLE);

  // Load OpenCL header from file.
  QFile headerFile(headerFilePath());
  if (headerFile.open(QIODevice::ReadOnly))
    OpenDataFile::infoTable.setGlobalMontageHeader(headerFile.readAll());

  // Check for any values in InfoTable that could make trouble.
  const int index = OpenDataFile::infoTable.getSelectedMontage();
  const int count = openDataFile->dataModel->montageTable()->rowCount();
  if (index < 0 || index >= count)
    OpenDataFile::infoTable.setSelectedMontage(0);

  // Pass the file to the child widgets.
  trackManager->changeFile(openDataFile.get());
  eventManager->changeFile(openDataFile.get());
  eventTypeManager->changeFile(openDataFile.get());
  montageManager->changeFile(openDataFile.get());
  filterManager->changeFile(openDataFile.get());
  videoPlayer->changeFile(openDataFile.get());

  syncDialog->changeFile(openDataFile.get());
  signalViewer->changeFile(openDataFile.get());

  // Update Filter tool bar.
  vector<double> comboNumbers{0, 5, 10};
  for (int i = 25; i <= fileResources->file->getSamplingFrequency() / 2; i *= 2)
    comboNumbers.push_back(i);

  double lpf = OpenDataFile::infoTable.getLowpassFrequency();
  bool lowpassOn = OpenDataFile::infoTable.getLowpassOn();
  if (lowpassOn && 0 < lpf &&
      lpf <= fileResources->file->getSamplingFrequency() / 2)
    comboNumbers.push_back(lpf);

  double hpf = OpenDataFile::infoTable.getHighpassFrequency();
  bool highpassOn = OpenDataFile::infoTable.getHighpassOn();
  if (highpassOn && 0 < hpf &&
      hpf <= fileResources->file->getSamplingFrequency() / 2)
    comboNumbers.push_back(hpf);

  sort(comboNumbers.begin(), comboNumbers.end());
  comboNumbers.erase(unique(comboNumbers.begin(), comboNumbers.end()),
                     comboNumbers.end());

  QStringList comboOptions("---");
  for (double e : comboNumbers)
    comboOptions << locale().toString(e, 'f', COMBO_PRECISION);

  QMetaObject::Connection c;

  lowpassComboBox->clear();
  lowpassComboBox->addItems(comboOptions);
  c = connect(lowpassComboBox, SIGNAL(currentIndexChanged(QString)), this,
              SLOT(lowpassComboBoxUpdate(QString)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)),
              this, SLOT(lowpassComboBoxUpdate(double)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassOnChanged(bool)), this,
              SLOT(lowpassComboBoxUpdate(bool)));
  openFileConnections.push_back(c);

  highpassComboBox->clear();
  highpassComboBox->addItems(comboOptions);
  c = connect(highpassComboBox, SIGNAL(currentIndexChanged(QString)), this,
              SLOT(highpassComboBoxUpdate(QString)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(highpassFrequencyChanged(double)), this,
              SLOT(highpassComboBoxUpdate(double)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(highpassOnChanged(bool)), this,
              SLOT(highpassComboBoxUpdate(bool)));
  openFileConnections.push_back(c);

  c = connect(notchCheckBox, SIGNAL(toggled(bool)), &OpenDataFile::infoTable,
              SLOT(setNotchOn(bool)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(notchOnChanged(bool)),
              notchCheckBox, SLOT(setChecked(bool)));
  openFileConnections.push_back(c);

  // Set up table models and managers.
  fileResources->eventTypeTable =
      make_unique<EventTypeTableModel>(openDataFile.get());
  eventTypeManager->setModel(fileResources->eventTypeTable.get());

  fileResources->montageTable =
      make_unique<MontageTableModel>(openDataFile.get());
  montageManager->setModel(fileResources->montageTable.get());

  fileResources->eventTable = make_unique<EventTableModel>(openDataFile.get());
  eventManager->setModel(fileResources->eventTable.get());

  fileResources->trackTable = make_unique<TrackTableModel>(openDataFile.get());
  trackManager->setModel(fileResources->trackTable.get());

  // Update the Select tool bar.
  auto cc = connectVitness(
      VitnessMontageTable::vitness(fileResources->dataModel->montageTable()),
      [this]() { updateMontageComboBox(); });
  openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
  updateMontageComboBox();

  auto indexChangedPtr =
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
  c = connect(montageComboBox, indexChangedPtr, [this](const int index) {
    const int count = fileResources->dataModel->montageTable()->rowCount();
    // If the index is out of range, ignore it. This can happen when deleting
    // the items in the montage select combo.
    if (0 <= index && index < count)
      OpenDataFile::infoTable.setSelectedMontage(index);
  });

  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)),
              montageComboBox, SLOT(setCurrentIndex(int)));
  openFileConnections.push_back(c);

  cc = connectVitness(VitnessEventTypeTable::vitness(
                          fileResources->dataModel->eventTypeTable()),
                      [this]() { updateEventTypeComboBox(); });
  openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
  updateEventTypeComboBox();

  c = connect(eventTypeComboBox, indexChangedPtr, [](int index) {
    OpenDataFile::infoTable.setSelectedType(index - 1);
  });
  openFileConnections.push_back(c);

  c = connect(
      &OpenDataFile::infoTable, &InfoTable::selectedTypeChanged,
      [this](int value) { eventTypeComboBox->setCurrentIndex(value + 1); });
  openFileConnections.push_back(c);

  vector<float> resolutionNumbers;
  stringstream ss(programOption<string>("resOptions"));
  while (ss) {
    float tmp;
    if (ss >> tmp)
      resolutionNumbers.push_back(tmp);
  }

  float sampleScaleValue = OpenDataFile::infoTable.getSampleScale();
  resolutionNumbers.push_back(sampleScaleValue);
  sort(resolutionNumbers.begin(), resolutionNumbers.end());
  resolutionNumbers.erase(
      unique(resolutionNumbers.begin(), resolutionNumbers.end()),
      resolutionNumbers.end());

  QStringList resolutionOptions;
  for (double e : resolutionNumbers)
    resolutionOptions << locale().toString(e, 'f', COMBO_PRECISION);
  resolutionComboBox->clear();
  resolutionComboBox->addItems(resolutionOptions);

  c = connect(resolutionComboBox, SIGNAL(currentIndexChanged(QString)), this,
              SLOT(resolutionComboBoxUpdate(QString)));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(sampleScaleChanged(float)), this,
              SLOT(resolutionComboBoxUpdate(float)));
  openFileConnections.push_back(c);

  // Update the status bar.
  QString str = "Start: " + sampleToDateTimeString(fileResources->file.get(), 0,
                                                   InfoTable::TimeMode::real);
  str += " Total time: " +
         sampleToDateTimeString(fileResources->file.get(),
                                fileResources->file->getSamplesRecorded(),
                                InfoTable::TimeMode::offset);
  timeStatusLabel->setText(str);

  c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)),
              this, SLOT(updatePositionStatusLabel()));
  openFileConnections.push_back(c);
  c = connect(signalViewer->getCanvas(),
              SIGNAL(cursorPositionSampleChanged(int)), this,
              SLOT(updateCursorStatusLabel()));
  openFileConnections.push_back(c);

  // Connect slot SignalViewer::updateSignalViewer() to make sure that the
  // SignalViewer gets updated when needed.
  // TODO: Perhaps move this block to signalViewer.
  c = connect(&OpenDataFile::infoTable, SIGNAL(virtualWidthChanged(int)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassOnChanged(bool)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(highpassFrequencyChanged(double)), signalViewer,
              SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(highpassOnChanged(bool)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(notchOnChanged(bool)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(filterWindowChanged(AlenkaSignal::WindowFunction)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(timeLineIntervalChanged(double)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersChanged()),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(frequencyMultipliersOnChanged(bool)), signalViewer,
              SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(sampleScaleChanged(float)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable, SIGNAL(sampleUnitsChanged(int)),
              signalViewer, SLOT(updateSignalViewer()));
  openFileConnections.push_back(c);
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(globalMontageHeaderChanged(QString)), signalViewer,
              SLOT(updateSignalViewer()));

  openFileConnections.push_back(c);

  cc = connectVitness(
      VitnessMontageTable::vitness(fileResources->dataModel->montageTable()),
      [this]() { signalViewer->updateSignalViewer(); });
  openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
  cc = connectVitness(VitnessEventTypeTable::vitness(
                          fileResources->dataModel->eventTypeTable()),
                      [this]() { signalViewer->updateSignalViewer(); });
  openFileConnections.insert(openFileConnections.end(), cc.begin(), cc.end());
  c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)),
              this, SLOT(updateManagers(int)));
  openFileConnections.push_back(c);

  // Update the View submenus.
  c = connect(&OpenDataFile::infoTable,
              SIGNAL(timeModeChanged(InfoTable::TimeMode)), this,
              SLOT(updateTimeMode(InfoTable::TimeMode)));
  c = connect(&OpenDataFile::infoTable, &InfoTable::timeLineIntervalChanged,
              [this](double value) {
                setTimeLineIntervalAction->setToolTip(
                    "The time line interval is " + locale().toString(value) +
                    " s");
                setTimeLineIntervalAction->setStatusTip(
                    setTimeLineIntervalAction->toolTip());
              });
  openFileConnections.push_back(c);

  // Load Elko session.
  const QString elkoSession = OpenDataFile::infoTable.getElkoSession();

  if (!elkoSession.isEmpty()) {
    QVariant returnValue, arg = elkoSession;
    QMetaObject::invokeMethod(view->rootObject(), "loadSession",
                              Q_RETURN_ARG(QVariant, returnValue),
                              Q_ARG(QVariant, arg));
  }

  // Emit all signals to ensure there are no uninitialized controls.
  OpenDataFile::infoTable.emitAllSignals();

  // Set up autosave.
  const int ms = 1000 * programOption<int>("autosave");

  if (ms > 0) {
    c = connect(autoSaveTimer, &QTimer::timeout, [this]() {
      try {
        if (undoStack->isClean())
          return;

        LocaleOverride::executeWithCLocale([this]() {
          fileResources->file->saveSecondaryFile(autoSaveName);
          logToFileAndConsole("Autosaving to " << autoSaveName);
        });
      } catch (const runtime_error &e) {
        errorMessage(this, catchDetailed(e));
      }
    });
    openFileConnections.push_back(c);

    autoSaveTimer->setInterval(ms);
    autoSaveTimer->start();
  }

  switchButton->setEnabled(true);
}

bool SignalFileBrowserWindow::closeFile() {
  if (!undoStack->isClean()) {
    auto res = QMessageBox::question(
        this, "Save File?", "Save changes before closing?",
        QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);

    if (res == QMessageBox::Save)
      saveFile();
    else if (res == QMessageBox::Cancel)
      return false;
  }

  logToFile("Closing file.");

  setWindowTitle(TITLE);
  undoStack->clear();
  setEnableFileActions(false);

  if (fileResources->file) {
    saveMontageHeader();

    try {
      LocaleOverride::executeWithCLocale([this]() {
        OpenDataFile::infoTable.writeXML(
            fileResources->file->getFilePath() + ".info",
            spikedetAnalysis->getSettings(), spikeDuration);
      });
    } catch (const runtime_error &e) {
      errorMessage(this, catchDetailed(e), "Error while autosaving file");
    }
  }

  OpenDataFile::infoTable.setDefaultValues();
  OpenDataFile::infoTable.emitAllSignals();

  deleteAutoSave();
  autoSaveName = "";

  closeFilePropagate();
  fileResources = make_unique<OpenFileResources>();
  signalViewer->updateSignalViewer();

  return true;
}

void SignalFileBrowserWindow::saveFile() {
  logToFile("Saving file.");

  if (fileResources->file) {
    try {
      LocaleOverride::executeWithCLocale(
          [this]() { fileResources->file->save(); });
    } catch (const runtime_error &e) {
      errorMessage(this, catchDetailed(e), "Error while saving file");
    }

    deleteAutoSave();

    undoStack->setClean();
    saveFileAction->setEnabled(false);
    autoSaveTimer->start();
  }
}

void SignalFileBrowserWindow::exportToEdf() {
  assert(fileResources->file);

  QFileInfo fileInfo(
      QString::fromStdString(fileResources->file->getFilePath()));
  QString fileName = QFileDialog::getSaveFileName(this, "Export to EDF file",
                                                  fileInfo.dir().absolutePath(),
                                                  "EDF files (*.edf)");

  if (fileName.isNull())
    return; // No file was selected.

  QFileInfo newFileInfo(fileName);
  if (newFileInfo.suffix() != "edf")
    fileName += ".edf";
  // TODO: Reject file names not with the proper suffix, rather
  // than add it
  // automatically.

  try {
    EDF::saveAs(fileName.toStdString(), fileResources->file.get());
  } catch (const runtime_error &e) {
    errorMessage(this, catchDetailed(e), "Error while exporting file");
  }
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(const QString &text) {
  if (fileResources->file) {
    bool ok;
    double value = locale().toDouble(text, &ok);

    if (ok) {
      if (OpenDataFile::infoTable.getLowpassFrequency() != value)
        OpenDataFile::infoTable.setLowpassFrequency(value);
      else
        setCurrentInNumericCombo(lowpassComboBox, value);
    }

    OpenDataFile::infoTable.setLowpassOn(ok);
  }
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(bool on) {
  if (fileResources->file) {
    if (on)
      lowpassComboBoxUpdate(OpenDataFile::infoTable.getLowpassFrequency());
    else
      lowpassComboBox->setCurrentIndex(0);
  }
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(double value) {
  if (fileResources->file) {
    if (value < 0 || value > fileResources->file->getSamplingFrequency() / 2)
      lowpassComboBoxUpdate(false);
    else
      setCurrentInNumericCombo(lowpassComboBox, value);
  }
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(const QString &text) {
  if (fileResources->file) {
    bool ok;
    double value = locale().toDouble(text, &ok);

    if (ok) {
      if (OpenDataFile::infoTable.getHighpassFrequency() != value)
        OpenDataFile::infoTable.setHighpassFrequency(value);
      else
        setCurrentInNumericCombo(highpassComboBox, value);
    }

    OpenDataFile::infoTable.setHighpassOn(ok);
  }
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(bool on) {
  if (fileResources->file) {
    if (on)
      highpassComboBoxUpdate(OpenDataFile::infoTable.getHighpassFrequency());
    else
      highpassComboBox->setCurrentIndex(0);
  }
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(double value) {
  if (fileResources->file) {
    if (value < 0 || value > fileResources->file->getSamplingFrequency() / 2)
      highpassComboBoxUpdate(false);
    else
      setCurrentInNumericCombo(highpassComboBox, value);
  }
}

void SignalFileBrowserWindow::resolutionComboBoxUpdate(const QString &text) {
  if (fileResources->file) {
    bool ok;
    float value = locale().toFloat(text, &ok);

    if (ok) {
      if (OpenDataFile::infoTable.getSampleScale() != value) {
        OpenDataFile::infoTable.setSampleScale(value);
      } else {
        setCurrentInNumericCombo(resolutionComboBox, value);
        sortInLastItem(resolutionComboBox);
      }
    }
  }
}

void SignalFileBrowserWindow::resolutionComboBoxUpdate(float value) {
  if (fileResources->file) {
    setCurrentInNumericCombo(resolutionComboBox, value);
    sortInLastItem(resolutionComboBox);
  }
}

void SignalFileBrowserWindow::updateManagers(int value) {
  for (auto e : managersConnections)
    disconnect(e);
  managersConnections.clear();

  const auto &mt = fileResources->dataModel->montageTable();
  // TODO: Try to eliminate calls to updateSignalViewer() from this class.

  if (0 < mt->rowCount()) {
    auto cc = connectVitness(VitnessTrackTable::vitness(mt->trackTable(value)),
                             [this]() { signalViewer->updateSignalViewer(); });
    managersConnections.insert(managersConnections.end(), cc.begin(), cc.end());

    cc = connectVitness(VitnessEventTable::vitness(mt->eventTable(value)),
                        [this]() { signalViewer->updateSignalViewer(); });
    managersConnections.insert(managersConnections.end(), cc.begin(), cc.end());
  }
}

void SignalFileBrowserWindow::updateTimeMode(InfoTable::TimeMode mode) {
  QAction *a = timeModeActionGroup->actions().at(static_cast<int>(mode));
  a->setChecked(true);

  timeModeStatusLabel->setText("Time Mode: " + a->text());
}

void SignalFileBrowserWindow::updatePositionStatusLabel() {
  const int position = OpenDataFile::infoTable.getPosition();
  const auto str = sampleToDateTimeString(fileResources->file.get(), position);
  positionStatusLabel->setText("Position: " + str);
}

void SignalFileBrowserWindow::updateCursorStatusLabel() {
  cursorStatusLabel->setText(
      "Cursor at: " +
      sampleToDateTimeString(
          fileResources->file.get(),
          signalViewer->getCanvas()->getCursorPositionSample()));
}

void SignalFileBrowserWindow::updateMontageComboBox() {
  if (fileResources->file) {
    const AbstractMontageTable *montageTable =
        openDataFile->dataModel->montageTable();
    int itemCount = montageComboBox->count();
    int selectedMontage = max(OpenDataFile::infoTable.getSelectedMontage(), 0);

    for (int i = 0; i < montageTable->rowCount(); ++i)
      montageComboBox->addItem(
          QString::fromStdString(montageTable->row(i).name));

    for (int i = 0; i < itemCount; ++i)
      montageComboBox->removeItem(0);

    const int index = min(selectedMontage, montageTable->rowCount() - 1);
    assert(0 <= index && index < montageTable->rowCount() &&
           "Make sure the selected index is legal");
    OpenDataFile::infoTable.setSelectedMontage(index);
  }
}

void SignalFileBrowserWindow::updateEventTypeComboBox() {
  if (fileResources->file) {
    const AbstractEventTypeTable *eventTypeTable =
        openDataFile->dataModel->eventTypeTable();
    int itemCount = eventTypeComboBox->count();
    int selectedType = OpenDataFile::infoTable.getSelectedType();

    eventTypeComboBox->addItem("<No Type>");
    for (int i = 0; i < eventTypeTable->rowCount(); ++i)
      eventTypeComboBox->addItem(
          QString::fromStdString(eventTypeTable->row(i).name));

    for (int i = 0; i < itemCount; ++i)
      eventTypeComboBox->removeItem(0);

    OpenDataFile::infoTable.setSelectedType(
        min(selectedType, eventTypeTable->rowCount() - 1));
  }
}

void SignalFileBrowserWindow::runSignalAnalysis(int i) {
  if (!fileResources->file)
    return;

  const AlenkaFile::AbstractMontageTable *montageTable =
      fileResources->dataModel->montageTable();
  if (montageTable->rowCount() <= 0)
    return;

  const AlenkaFile::AbstractTrackTable *trackTable =
      montageTable->trackTable(OpenDataFile::infoTable.getSelectedMontage());
  if (trackTable->rowCount() <= 0)
    return;

  // Set some details in the analysis objects.
  spikedetAnalysis->setSpikeDuration(spikeDuration);
  modifiedSpikedetAnalysis->setSpikeDuration(spikeDuration);
  clusterAnalysis->setSpikeDuration(spikeDuration);
  centeringClusterAnalysis->setSpikeDuration(spikeDuration);

  // Run the appropriate analysis.
  signalAnalysis[i]->runAnalysis(openDataFile.get(), this);

  // Send Spikedet result to Cluster.
  CDischarges *discharges = nullptr;

  if (signalAnalysis[i].get() == spikedetAnalysis) {
    discharges = spikedetAnalysis->getDischarges();
  } else if (signalAnalysis[i].get() == modifiedSpikedetAnalysis) {
    discharges = modifiedSpikedetAnalysis->getDischarges();
  }

  if (discharges) {
    clusterAnalysis->setDischarges(discharges);
    centeringClusterAnalysis->setDischarges(discharges);
  }
}

void SignalFileBrowserWindow::cleanChanged(bool clean) {
  saveFileAction->setEnabled(!clean);
}

void SignalFileBrowserWindow::closeFilePropagate() {
  for (auto e : openFileConnections)
    disconnect(e);
  openFileConnections.clear();

  montageComboBox->clear();
  eventTypeComboBox->clear();
  switchButton->setEnabled(false);

  trackManager->changeFile(nullptr);
  eventManager->changeFile(nullptr);
  eventTypeManager->changeFile(nullptr);
  montageManager->changeFile(nullptr);
  filterManager->changeFile(nullptr);
  videoPlayer->changeFile(nullptr);

  syncDialog->changeFile(nullptr);
  signalViewer->changeFile(nullptr);
}

void SignalFileBrowserWindow::setEnableFileActions(bool enable) {
  closeFileAction->setEnabled(enable);
  exportToEdfAction->setEnabled(enable);

  for (auto &e : analysisActions)
    e->setEnabled(enable);
}

void SignalFileBrowserWindow::setFilePathInQML() {
  if (fileResources->file) {
    string fileName = autoSaveName + to_string(nameIndex++ % 2);
    LocaleOverride::executeWithCLocale([this, fileName]() {
      fileResources->file->saveSecondaryFile(fileName);
      logToFileAndConsole("Autosaving to " << fileName);
    });

    QFileInfo fileInfo = QString::fromStdString(fileName);
    QString filePath = fileInfo.absoluteFilePath();

    view->rootContext()->setContextProperty(
        "filePath", QVariant::fromValue("file:///" + filePath));
  } else {
    view->rootContext()->setContextProperty(
        "filePath", QVariant::fromValue(QStringLiteral("")));
  }
}

void SignalFileBrowserWindow::switchToAlenka() {
  logToFile("Switching to Alenka.");
  signalViewer->getCanvas()->setPaintingDisabled(false);

  stackedWidget->setCurrentIndex(1);

  restoreState(windowState);
  windowState.clear();
  windowGeometry.clear();

  menuBar()->show();
  statusBar()->show();

  string mode;
  programOption("mode", mode);

  if (mode == "tablet" || mode == "tablet-full")
    showMaximized();
}

void SignalFileBrowserWindow::verticalZoomIn() {
  int index = resolutionComboBox->currentIndex() - 1;

  if (0 <= index) {
    resolutionComboBox->setCurrentIndex(index);
  } else {
    index = unitsComboBox->currentIndex() - 1;

    if (0 <= index) {
      unitsComboBox->setCurrentIndex(index);
      resolutionComboBox->setCurrentIndex(resolutionComboBox->count() - 1);
    }
  }
}

void SignalFileBrowserWindow::verticalZoomOut() {
  int index = resolutionComboBox->currentIndex() + 1;

  if (index < resolutionComboBox->count()) {
    resolutionComboBox->setCurrentIndex(index);
  } else {
    index = unitsComboBox->currentIndex() + 1;

    if (index < unitsComboBox->count()) {
      unitsComboBox->setCurrentIndex(index);
      resolutionComboBox->setCurrentIndex(0);
    }
  }
}

void SignalFileBrowserWindow::exportDialog() {
  QString picutres;

  if (isProgramOptionSet("screenPath")) {
    picutres = QString::fromStdString(programOption<string>("screenPath"));
  } else {
    auto pathList =
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (pathList.size() > 0)
      picutres = pathList.at(0);
    else
      runtime_error("Cannot find pictures dir.");
  }

  QString type = QString::fromStdString(programOption<string>("screenType"));
  QString baseName =
      QFileInfo(QString::fromStdString(fileResources->file->getFilePath()))
          .baseName();
  int i = 0;

  while (1) {
    QFileInfo fileInfo(picutres + QDir::separator() + baseName + "-" +
                       QString::number(i++) + "." + type);

    if (!fileInfo.exists()) {
      QVariant returnValue, arg = fileInfo.absoluteFilePath();
      QMetaObject::invokeMethod(view->rootObject(), "takeScreenshot",
                                Q_RETURN_ARG(QVariant, returnValue),
                                Q_ARG(QVariant, arg));
      return;
    }
  }
}
