#include "signalviewer.h"

#include "../Alenka-File/include/AlenkaFile/datafile.h"
#include "DataModel/opendatafile.h"
#include "canvas.h"
#include "options.h"
#include "tracklabelbar.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

using namespace std;

namespace {

//! How many mouse wheel changes does it take to move one screen to the right.
const int SCROLLBAR_STEP = 20;

} // namespace

SignalViewer::SignalViewer(QWidget *parent) : QWidget(parent) {
  auto box = new QVBoxLayout;
  setLayout(box);
  box->setContentsMargins(0, 0, 0, 0);
  box->setSpacing(0);

  splitter = new QSplitter;
  box->addWidget(splitter);

  trackLabelBar = new TrackLabelBar(this);
  splitter->addWidget(trackLabelBar);
  splitter->setStretchFactor(0, 0);

  canvas = new Canvas(this);
  splitter->addWidget(canvas);
  splitter->setStretchFactor(1, 1);

  scrollBar = new QScrollBar(Qt::Horizontal, this);
  string mode;
  programOption("mode", mode);
  if (mode == "tablet" || mode == "tablet-full")
    scrollBar->setMinimumHeight(50);
  box->addWidget(scrollBar);

  splitter->restoreState(
      PROGRAM_OPTIONS->settings("SignalViewer splitter state").toByteArray());
}

SignalViewer::~SignalViewer() {
  PROGRAM_OPTIONS->settings("SignalViewer splitter state",
                            splitter->saveState());
}

void SignalViewer::changeFile(OpenDataFile *const file) {
  canvas->changeFile(file);
  trackLabelBar->changeFile(file);

  this->file = file;

  for (auto e : openFileConnections)
    disconnect(e);
  openFileConnections.clear();

  if (file) {
    auto c = connect(scrollBar, SIGNAL(valueChanged(int)),
                     &OpenDataFile::infoTable, SLOT(setPosition(int)));
    openFileConnections.push_back(c);

    c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)),
                scrollBar, SLOT(setValue(int)));
    openFileConnections.push_back(c);

    c = connect(&OpenDataFile::infoTable, SIGNAL(virtualWidthChanged(int)),
                this, SLOT(resize()));
    openFileConnections.push_back(c);

    c = connect(canvas, SIGNAL(cursorPositionTrackChanged(int)), trackLabelBar,
                SLOT(setSelectedTrack(int)));
    openFileConnections.push_back(c);

    scrollBar->setRange(0, file->file->getSamplesRecorded());
    resize();
  }
}

void SignalViewer::updateSignalViewer() {
  canvas->update(); // For some reason in Qt 5.7 the child canvas doesn't get
                    // updated. So I do it here explicitly.
  update();
}

void SignalViewer::resizeEvent(QResizeEvent * /*event*/) { resize(); }

void SignalViewer::wheelEvent(QWheelEvent *event) {
  scrollBar->event(reinterpret_cast<QEvent *>(event));
}

// Perhaps move catching of these events to the parent window, so that the user
// doesn't have to always have focus on the canvas.
void SignalViewer::keyPressEvent(QKeyEvent *event) {
  if (Qt::Key_Space == event->key())
    event->ignore(); // Propagate the event up so that we can play/pause video.
  else
    scrollBar->event(reinterpret_cast<QEvent *>(event));
}

void SignalViewer::keyReleaseEvent(QKeyEvent *event) {
  scrollBar->event(reinterpret_cast<QEvent *>(event));
}

void SignalViewer::resize() {
  if (file) {
    const int pageWidth = canvas->width();
    const double ratio = static_cast<double>(file->file->getSamplesRecorded()) /
                         OpenDataFile::infoTable.getVirtualWidth();
    const int samplesPerPage = static_cast<int>(pageWidth * ratio);

    scrollBar->setPageStep(samplesPerPage);
    scrollBar->setSingleStep(max(1, samplesPerPage / SCROLLBAR_STEP));

    OpenDataFile::infoTable.setPixelViewWidth(pageWidth); // TODO: check this

    canvas->update(); // TODO: Figure out whether this is necessary.
  }
}
