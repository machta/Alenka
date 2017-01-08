#include "signalviewer.h"

#include "canvas.h"
#include "options.h"
#include "DataFile/datafile.h"
#include "tracklabelbar.h"

#include <QVBoxLayout>
#include <QSplitter>

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

void SignalViewer::changeFile(DataFile* file)
{
	canvas->changeFile(file);
	trackLabelBar->changeFile(file);

	if (file != nullptr)
	{
		infoTable = file->getInfoTable();

		connect(scrollBar, SIGNAL(valueChanged(int)), infoTable, SLOT(setPosition(int)));
		connect(infoTable, SIGNAL(positionChanged(int)), this, SLOT(setPosition(int)));

		connect(infoTable, SIGNAL(virtualWidthChanged(int)), this, SLOT(setVirtualWidth(int)));

		connect(canvas, SIGNAL(cursorPositionTrackChanged(int)), trackLabelBar, SLOT(setSelectedTrack(int)));
	}
	else
	{
		infoTable = nullptr;
	}
}

void SignalViewer::resize(int virtualWidth)
{
	int page = canvas->width();

	scrollBar->setRange(0, virtualWidth - page - 1);
	scrollBar->setPageStep(page);
	scrollBar->setSingleStep(std::max(1, page/20));
}

void SignalViewer::setVirtualWidth(int value)
{
	// This version mantains the position of the left canvas edge.
//	double relPosition = scrollBar->value();
//	relPosition /= virtualWidthFromScrollBar();

//	resize(value);

//	setPosition(std::round(relPosition*value));

//	canvas->update();

	// This version makes sure that the position indicator stays at the same time.
	double oldPosition = scrollBar->value() + getInfoTable()->getPositionIndicator()*canvas->width();
	double ratio = static_cast<double>(value)/virtualWidthFromScrollBar();

	resize(value);

	setPosition(std::round(oldPosition*ratio - getInfoTable()->getPositionIndicator()*canvas->width()));

	canvas->update();
}

void SignalViewer::setPosition(int value)
{
	scrollBar->setValue(value);

	canvas->update();
}
