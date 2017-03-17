#include "signalviewer.h"

#include "canvas.h"
#include "options.h"
#include <AlenkaFile/datafile.h>
#include "tracklabelbar.h"
#include "signalfilebrowserwindow.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <algorithm>
#include <cmath>

using namespace std;

SignalViewer::SignalViewer(QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* box = new QVBoxLayout;
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
	box->addWidget(scrollBar);

	splitter->restoreState(PROGRAM_OPTIONS.settings("SignalViewer splitter state").toByteArray());
}

SignalViewer::~SignalViewer()
{
	PROGRAM_OPTIONS.settings("SignalViewer splitter state", splitter->saveState());
}

void SignalViewer::changeFile(AlenkaFile::DataFile* file)
{
	canvas->changeFile(file);
	trackLabelBar->changeFile(file);

	this->file = file;

	if (file)
	{
		auto c = connect(scrollBar, SIGNAL(valueChanged(int)), &SignalFileBrowserWindow::infoTable, SLOT(setPosition(int)));
		openFileConnections.push_back(c);
		c = connect(&SignalFileBrowserWindow::infoTable, SIGNAL(positionChanged(int)), this, SLOT(setPosition(int)));
		openFileConnections.push_back(c);

		c = connect(&SignalFileBrowserWindow::infoTable, SIGNAL(virtualWidthChanged(int)), this, SLOT(setVirtualWidth(int)));
		openFileConnections.push_back(c);

		c = connect(canvas, SIGNAL(cursorPositionTrackChanged(int)), trackLabelBar, SLOT(setSelectedTrack(int)));
		openFileConnections.push_back(c);
	}
	else
	{
		for (auto e : openFileConnections)
			disconnect(e);
		openFileConnections.clear();
	}
}

void SignalViewer::updateSignalViewer()
{
	canvas->update(); // For some reason in Qt 5.7 the child canvas doesn't get updated. So I do it here explicitly.
	update();
}

void SignalViewer::resizeEvent(QResizeEvent*)
{
	resize(SignalFileBrowserWindow::infoTable.getVirtualWidth());
}

void SignalViewer::wheelEvent(QWheelEvent* event)
{
	scrollBar->event(reinterpret_cast<QEvent*>(event));
}

void SignalViewer::keyPressEvent(QKeyEvent* event)
{
	scrollBar->event(reinterpret_cast<QEvent*>(event));
}

void SignalViewer::keyReleaseEvent(QKeyEvent* event)
{
	scrollBar->event(reinterpret_cast<QEvent*>(event));
}

void SignalViewer::resize(int virtualWidth)
{
	int pageWidth = canvas->width();

	scrollBar->setRange(0, virtualWidth - pageWidth - 1);
	scrollBar->setPageStep(pageWidth);
	scrollBar->setSingleStep(max(1, pageWidth/20));
}

int SignalViewer::virtualWidthFromScrollBar()
{
	return scrollBar->maximum() + scrollBar->pageStep() + 1 - scrollBar->minimum();
}

void SignalViewer::setVirtualWidth(int value)
{
	// This version makes sure that the position indicator stays at the same time.
	double oldPosition = scrollBar->value() + SignalFileBrowserWindow::infoTable.getPositionIndicator()*canvas->width();
	double ratio = static_cast<double>(value)/virtualWidthFromScrollBar();

	resize(value);
	setPosition(round(oldPosition*ratio - SignalFileBrowserWindow::infoTable.getPositionIndicator()*canvas->width()));

	canvas->update();
}

void SignalViewer::setPosition(int value)
{
	scrollBar->setValue(value);
	canvas->update();
}
