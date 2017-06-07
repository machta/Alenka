#include "signalviewer.h"

#include "../Alenka-File/include/AlenkaFile/datafile.h"
#include "DataModel/opendatafile.h"
#include "canvas.h"
#include "options.h"
#include "tracklabelbar.h"

#include <QScrollBar>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

using namespace std;

SignalViewer::SignalViewer(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *box = new QVBoxLayout;
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
  if (PROGRAM_OPTIONS["mode"].as<string>() == "tablet" ||
      PROGRAM_OPTIONS["mode"].as<string>() == "tablet-full")
    scrollBar->setMinimumHeight(50);
  box->addWidget(scrollBar);

  splitter->restoreState(
      PROGRAM_OPTIONS.settings("SignalViewer splitter state").toByteArray());
}

SignalViewer::~SignalViewer() {
  SET_PROGRAM_OPTIONS.settings("SignalViewer splitter state",
                               splitter->saveState());
}

void SignalViewer::changeFile(OpenDataFile *file) {
  canvas->changeFile(file);
  trackLabelBar->changeFile(file);

  this->file = file;

  if (file) {
    auto c = connect(scrollBar, SIGNAL(valueChanged(int)),
                     &OpenDataFile::infoTable, SLOT(setPosition(int)));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this,
                SLOT(setPosition(int)));
    openFileConnections.push_back(c);

    c = connect(&OpenDataFile::infoTable, SIGNAL(virtualWidthChanged(int)),
                this, SLOT(setVirtualWidth(int)));
    openFileConnections.push_back(c);

    c = connect(canvas, SIGNAL(cursorPositionTrackChanged(int)), trackLabelBar,
                SLOT(setSelectedTrack(int)));
    openFileConnections.push_back(c);
  } else {
    for (auto e : openFileConnections)
      disconnect(e);
    openFileConnections.clear();
  }
}

void SignalViewer::updateSignalViewer() {
  canvas->update(); // For some reason in Qt 5.7 the child canvas doesn't get
                    // updated. So I do it here explicitly.
  update();
}

void SignalViewer::resizeEvent(QResizeEvent *event) {
  (void)event;
  resize(OpenDataFile::infoTable.getVirtualWidth());
}

void SignalViewer::wheelEvent(QWheelEvent *event) {
  scrollBar->event(reinterpret_cast<QEvent *>(event));
}

void SignalViewer::keyPressEvent(QKeyEvent *event) {
  scrollBar->event(reinterpret_cast<QEvent *>(event));
}

void SignalViewer::keyReleaseEvent(QKeyEvent *event) {
  scrollBar->event(reinterpret_cast<QEvent *>(event));
}

void SignalViewer::resize(int virtualWidth) {
  const int pageWidth = canvas->width();

  scrollBar->setRange(0, virtualWidth - pageWidth - 1);
  scrollBar->setPageStep(pageWidth);
  scrollBar->setSingleStep(max(1, pageWidth / 20));

  OpenDataFile::infoTable.setPixelViewWidth(pageWidth);
}

int SignalViewer::virtualWidthFromScrollBar() {
  return scrollBar->maximum() + scrollBar->pageStep() + 1 -
         scrollBar->minimum();
}

void SignalViewer::setVirtualWidth(int value) {
  // This version makes sure that the position indicator stays at the same time.
  const double oldPosition =
      scrollBar->value() +
      OpenDataFile::infoTable.getPositionIndicator() * canvas->width();
  const double ratio = static_cast<double>(value) / virtualWidthFromScrollBar();

  resize(value);
  setPosition(
      round(oldPosition * ratio -
            OpenDataFile::infoTable.getPositionIndicator() * canvas->width()));

  canvas->update();
}

void SignalViewer::setPosition(int value) {
  scrollBar->setValue(value);
  canvas->update();
}
