#include "tracklabelbar.h"

#include "DataModel/opendatafile.h"
#include "options.h"
#include <AlenkaFile/datafile.h>
#include "DataModel/vitnessdatamodel.h"
#include "canvas.h"

#include <QPainter>

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

TrackLabelBar::TrackLabelBar(QWidget* parent) : QWidget(parent)
{
	setMinimumWidth(0);
	setMaximumWidth(300);

	connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateTrackTable(int)));
}

void TrackLabelBar::changeFile(OpenDataFile* file)
{
	this->file = file;
}

void TrackLabelBar::paintEvent(QPaintEvent* /*event*/)
{
	if (!file)
		return;

	AbstractTrackTable* trackTable = file->dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());

	int totalTracks = 0;
	for (int i = 0; i < trackTable->rowCount(); ++i)
	{
		if (trackTable->row(i).hidden == false)
			++totalTracks;
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
		Track t = trackTable->row(track + hidden);

		if (t.hidden == false)
		{
			QColor color = DataModel::array2color<QColor>(t.color);
			if (track == selectedTrack)
				color = Canvas::modifySelectionColor(color);
			painter.setPen(color);

			double y = (0.5 + track)*rowHeight + fontHeight/2;
			double x = 2;

			painter.drawText(QPointF(x, y), QString::fromStdString(t.label));

			++track;
		}
		else
		{
			++hidden;
		}
	}
}

void TrackLabelBar::updateTrackTable(int row)
{
	if (file && row >= 0)
	{
		for (auto e : trackConnections)
			disconnect(e);
		trackConnections.clear();

		auto vitness = VitnessTrackTable::vitness(file->dataModel->montageTable()->trackTable(row));

		auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(updateLabels(int)));
		trackConnections.push_back(c);
		c = connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(update()));
		trackConnections.push_back(c);
		c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this, SLOT(update()));
		trackConnections.push_back(c);
	}
}

void TrackLabelBar::updateLabels(int col)
{
	if (col == static_cast<int>(Track::Index::label))
		update();
}
