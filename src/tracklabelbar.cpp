#include "tracklabelbar.h"

#include "../Alenka-File/include/AlenkaFile/datafile.h"
#include "DataModel/opendatafile.h"
#include "DataModel/vitnessdatamodel.h"
#include "canvas.h"
#include "options.h"

#include <QPainter>

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace {

const double EPS = 0.001;
const QChar ARROW_UP(0x2191);
const QChar ARROW_DOWN(0x2193);

enum amplitudeArrow { noArrow, upArrow, downArrow };

const auto compareNear = [](double a, double b) {
  if (abs((a - b) / max(abs(a), abs(b))) < EPS)
    return false;
  else
    return a < b;
};

// O(n*log(n)) version.
double computeMostCommonValue(const vector<double> &array) {
  assert(0 < array.size());

  vector<double> sortedArray = array;
  sort(sortedArray.begin(), sortedArray.end());

  int maxCount = 1;
  int lastCount = 1;
  double maxValue = sortedArray[0];
  double lastValue = sortedArray[0];

  for (unsigned int i = 1; i < sortedArray.size(); ++i) {
    double value = array[i];

    if (!compareNear(value, lastValue) && !compareNear(lastValue, value)) {
      ++lastCount;

      if (maxCount < lastCount) {
        maxCount = lastCount;
        maxValue = lastValue;
      }
    } else {
      lastCount = 1;
      lastValue = value;
    }
  }

  return maxValue;
}

vector<int> computeAmplitudeArrows(const vector<double> &amps) {
  double mostCommonValue = computeMostCommonValue(amps);
  vector<int> arrows;

  bool negative = mostCommonValue < 0;
  if (negative)
    mostCommonValue *= -1;

  for (double value : amps) {
    if (negative)
      value *= -1;
    int arrow;

    if (compareNear(value, mostCommonValue))
      arrow = downArrow;
    else if (compareNear(mostCommonValue, value))
      arrow = upArrow;
    else
      arrow = noArrow;

    arrows.push_back(arrow);
  }

  return arrows;
}

} // namespace

TrackLabelBar::TrackLabelBar(QWidget *parent) : QWidget(parent) {
  setMinimumWidth(0);
  setMaximumWidth(300);

  connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this,
          SLOT(updateConnections(int)));
}

void TrackLabelBar::paintEvent(QPaintEvent * /*event*/) {
  assert(labels.size() == colors.size() &&
         labels.size() == amplitudeArrows.size());

  int totalTracks = static_cast<int>(labels.size());
  double rowHeight = static_cast<double>(height()) / totalTracks;
  double fontHeight = min<double>(10, rowHeight / 1.5);

  QPainter painter(this);
  QFont font = painter.font();
  font.setPointSizeF(fontHeight);
  painter.setFont(font);

  QFontMetrics fm(font);
  int pixelOffset = drawArrows ? fm.width(ARROW_UP) : 2;

  for (int i = 0; i < totalTracks; ++i) {
    double y = (0.5 + i) * rowHeight + fontHeight / 2;
    QPointF point(0, y);

    QColor color;

    if (drawArrows && amplitudeArrows[i] != noArrow) {
      QChar arrowChar;

      if (amplitudeArrows[i] == upArrow) {
        arrowChar = ARROW_UP;
        color = QColor(255, 0, 0);
      } else {
        arrowChar = ARROW_DOWN;
        color = QColor(0, 255, 0);
      }

      painter.setPen(color);
      painter.drawText(point, arrowChar);
    }

    point.setX(point.x() + pixelOffset);

    color = colors[i];
    if (i == selectedTrack)
      color = Canvas::modifySelectionColor(color);

    painter.setPen(color);
    painter.drawText(point, labels[i]);
  }
}

void TrackLabelBar::updateConnections(int row) {
  for (auto e : trackConnections)
    disconnect(e);
  trackConnections.clear();

  if (file) {
    const AbstractMontageTable *mt = file->dataModel->montageTable();

    if (0 <= row && row < mt->rowCount()) {
      auto vitness = VitnessTrackTable::vitness(mt->trackTable(row));

      auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this,
                       SLOT(updateLabels()));
      trackConnections.push_back(c);
      c = connect(vitness, SIGNAL(rowsInserted(int, int)), this,
                  SLOT(updateLabels()));
      trackConnections.push_back(c);
      c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
                  SLOT(updateLabels()));
      trackConnections.push_back(c);

      updateLabels();
    }
  }
}

void TrackLabelBar::updateLabels() {
  labels.clear();
  colors.clear();
  amplitudeArrows.clear();

  if (!file || file->dataModel->montageTable()->rowCount() <= 0)
    return;

  const AbstractTrackTable *trackTable =
      file->dataModel->montageTable()->trackTable(
          OpenDataFile::infoTable.getSelectedMontage());

  if (trackTable->rowCount() <= 0)
    return;

  int hidden = 0;
  int track = 0;
  vector<double> amps;

  for (int i = 0; i < trackTable->rowCount(); ++i) {
    assert(track + hidden == i);
    Track t = trackTable->row(i);

    if (t.hidden == false) {
      labels.push_back(QString::fromStdString(t.label));
      colors.push_back(DataModel::array2color<QColor>(t.color));
      amps.push_back(t.amplitude);

      ++track;
    } else {
      ++hidden;
    }
  }

  amplitudeArrows = computeAmplitudeArrows(amps);

  unsigned int noArrowCount = 0;
  for (int e : amplitudeArrows) {
    if (e == noArrow)
      ++noArrowCount;
  }
  drawArrows = noArrowCount != amplitudeArrows.size();

  update();
}
