#include "tracklabelbar.h"

#include "options.h"
#include "DataFile/datafile.h"
#include "canvas.h"

#include <QPainter>

#include <algorithm>

using namespace std;

TrackLabelBar::TrackLabelBar(QWidget* parent) : QWidget(parent)
{
	setMinimumWidth(0);
	setMaximumWidth(300);
}

TrackLabelBar::~TrackLabelBar()
{
}

void TrackLabelBar::changeFile(DataFile* file)
{
	if (file != nullptr)
	{
		infoTable = file->getInfoTable();
		montageTable = file->getMontageTable();

		connect(infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateTrackTable()));
	}
	else
	{
		infoTable = nullptr;
		montageTable = nullptr;
	}
}

void TrackLabelBar::paintEvent(QPaintEvent* /*event*/)
{
	if (montageTable == nullptr)
	{
		return;
	}

	int totalTracks = 0;
	for (int i = 0; i < trackTable->rowCount(); ++i)
	{
		if (trackTable->getHidden(i) == false)
		{
			++totalTracks;
		}
	}

	double rowHeight = static_cast<double>(height())/totalTracks;
	double fontHeight = min<double>(10, rowHeight/1.5);

	QPainter painter(this);
	QFont font = painter.font();
	font.setPointSizeF(fontHeight);
	painter.setFont(font);

	int hidden = 0;
	int track = 0;

	while (track < totalTracks)
	{
		if (trackTable->getHidden(track + hidden) == false)
		{
			QColor color = trackTable->getColor(track + hidden);
			if (track == selectedTrack)
			{
				color = Canvas::modifySelectionColor(color);
			}
			painter.setPen(color);

			double y = (0.5 + track)*rowHeight + fontHeight/2;
			double x = 2;

			painter.drawText(QPointF(x, y), QString::fromStdString(trackTable->getLabel(track + hidden)));

			++track;
		}
		else
		{
			++hidden;
		}
	}
}
