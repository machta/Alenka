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

	QPainter painter(this);
	//painter.setFont(QFont("Arial", 30));
	//painter.drawText(rect(), Qt::AlignCenter, "Qt");

	int hidden = 0;
	for (int track = 0; track < totalTracks; ++track)
	{
		if (trackTable->getHidden(track + hidden) == false)
		{
			QColor color = trackTable->getColor(track + hidden);
			if (track == selectedTrack)
			{
				color = Canvas::modifySelectionColor(color);
			}
			painter.setPen(color);

			double y = (0.5 + track)*height()/totalTracks;
			double x = 1;

			painter.drawText(QPointF(x, y), QString::fromStdString(trackTable->getLabel(track + hidden)));
		}
		else
		{
			++hidden;
		}
	}
}
