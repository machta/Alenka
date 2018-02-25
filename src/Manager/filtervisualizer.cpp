#include "filtervisualizer.h"

#include "../DataModel/opendatafile.h"

#include <QAction>
#include <QVBoxLayout>
#include <QtCharts>

#include <cmath>
#include <complex>

using namespace std;
using namespace QtCharts;

namespace {

const float PEN_WIDTH = 1;
const int POINTS_PER_FREQUENCY = 16;

vector<float> computeFilterResponse(const vector<float> &bb,
                                    const unsigned int n,
                                    Eigen::FFT<float> *fft) {
  assert(bb.size() <= n);

  vector<float> b = bb;
  b.resize(n, 0);

  vector<complex<float>> response;
  fft->fwd(response, b);

  vector<float> absResponse(n);
  for (unsigned int i = 0; i < n; ++i)
    absResponse[i] = 20 * log10(abs(response[i]));

  return absResponse;
}

} // namespace

FilterVisualizer::FilterVisualizer(QWidget *parent)
    : QWidget(parent), fft(new Eigen::FFT<float>()) {
  chartView = new QChartView(this);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setDragMode(QGraphicsView::NoDrag);

  resetAction = new QAction(QIcon(":/icons/fit.png"), "Reset view", this);
  connect(resetAction, &QAction::triggered, [this]() { chart->zoomReset(); });
  chartView->addAction(resetAction);

  zoomAction = new QAction(QIcon(":/icons/zoom-in.png"), "Zoom", this);
  zoomAction->setToolTip("Drag to zoom in, right-click to zoom out");
  zoomAction->setCheckable(true);
  connect(zoomAction, &QAction::toggled, [this](bool enable) {
    chartView->setRubberBand(enable ? QChartView::RectangleRubberBand
                                    : QChartView::NoRubberBand);
  });
  chartView->addAction(zoomAction);

  auto box = new QVBoxLayout();
  box->setMargin(0);
  box->addWidget(chartView);
  setLayout(box);

  // Set up chart.
  chart = new QChart();
  chart->legend()->hide();
  chartView->setChart(chart);

  axisX = new QValueAxis();
  chart->addAxis(axisX, Qt::AlignBottom);

  axisSpectrum = new QValueAxis();
  axisSpectrum->setTitleText("Amplitude");
  chart->addAxis(axisSpectrum, Qt::AlignLeft);

  spectrumSeries = new QLineSeries();
  chart->addSeries(spectrumSeries);
  spectrumSeries->attachAxis(axisX);
  spectrumSeries->attachAxis(axisSpectrum);
  QPen pen = spectrumSeries->pen();
  pen.setWidthF(PEN_WIDTH);
  spectrumSeries->setPen(pen);

  axisResponse = new QValueAxis();
  axisResponse->setTitleText("Response (dB)");
  chart->addAxis(axisResponse, Qt::AlignRight);

  responseSeries = new QLineSeries();
  chart->addSeries(responseSeries);
  responseSeries->attachAxis(axisX);
  responseSeries->attachAxis(axisResponse);
  pen = responseSeries->pen();
  pen.setWidthF(PEN_WIDTH);
  responseSeries->setPen(pen);
}

void FilterVisualizer::changeFile(OpenDataFile *file) {
  this->file = file;

  for (auto e : connections)
    disconnect(e);
  connections.clear();

  if (file) {
    auto c = connect(&file->infoTable, SIGNAL(filterCoefficientsChanged()),
                     this, SLOT(updateResponse()));
    connections.push_back(c);

    c = connect(&file->infoTable, SIGNAL(positionChanged(int, double)), this,
                SLOT(updateSpectrum()));
    connections.push_back(c);

    c = connect(&file->infoTable, SIGNAL(pixelViewWidthChanged(int)),
                SLOT(updateSpectrum()));
    connections.push_back(c);
  }

  forceUpdateSpectrum();
}

void FilterVisualizer::addSeries() {
  chart->addSeries(spectrumSeries);
  chart->addSeries(responseSeries);
}

void FilterVisualizer::removeSeries() {
  chart->removeSeries(spectrumSeries);
  chart->removeSeries(responseSeries);
}

void FilterVisualizer::updateSpectrum() {
  if (freezeSpectrum)
    return;

  spectrumSeries->clear();
  if (!file || secondToDisplay == 0)
    return;

  assert(channelToDisplay < static_cast<int>(file->file->getChannelCount()));

  removeSeries();

  const double fs = file->file->getSamplingFrequency() / 2;
  axisX->setRange(0, fs);

  const int samplesToUse =
      secondToDisplay * static_cast<int>(file->file->getSamplingFrequency());
  const int position = OpenDataFile::infoTable.getPosition();

  const int start = max(0, position - samplesToUse / 2);
  const int end = start + samplesToUse - 1;
  const int sampleCount =
      end - start + 1; // TODO: Maybe round this to a power of two.

  buffer.resize(sampleCount * file->file->getChannelCount());
  file->file->readSignal(buffer.data(), start, end);

  auto begin = buffer.begin() + channelToDisplay * sampleCount;
  vector<float> input(begin, begin + sampleCount);

  vector<complex<float>> spectrum;
  fft->fwd(spectrum, input);

  assert(static_cast<int>(input.size()) == sampleCount);
  assert(static_cast<int>(spectrum.size()) == sampleCount);

  float maxVal = 0;

  for (int i = 0; i < sampleCount / 2; ++i) {
    float val = abs(spectrum[i]);
    if (isfinite(val)) {
      maxVal = max(maxVal, val);
      spectrumSeries->append(fs * i / (sampleCount / 2), val);
    }
  }

  axisSpectrum->setRange(0, maxVal);
  addSeries();
}

void FilterVisualizer::updateResponse() {
  responseSeries->clear();
  if (!file)
    return;

  removeSeries();

  const double fs = file->file->getSamplingFrequency() / 2;
  axisX->setRange(0, fs);

  vector<float> response = computeFilterResponse(
      OpenDataFile::infoTable.getFilterCoefficients(),
      qNextPowerOfTwo(static_cast<int>(fs) * 2 * POINTS_PER_FREQUENCY),
      fft.get());

  float minVal = 1000 * 1000 * 1000;
  float maxVal = 0;
  const int n2 = static_cast<int>(response.size()) / 2;

  for (int i = 0; i < n2; ++i) {
    float val = response[i];
    if (isfinite(val)) {
      maxVal = max(maxVal, val);
      minVal = min(minVal, val);
    }
  }

  float diff = abs(maxVal - minVal);
  if (diff < 0.001) {
    // Draw a straight line (e.g. when using an all-pass filter).
    responseSeries->append(0, maxVal);
    responseSeries->append(1, maxVal);

    // One more point needs to be added, otherwise Qt draws nothing at all.
    responseSeries->append(1, abs(maxVal) < 0.001 ? maxVal - 1 : 0);
  } else {
    for (int i = 0; i < n2; ++i) {
      float val = response[i];
      if (isfinite(val))
        responseSeries->append(fs * i / n2, val);
    }
  }

  axisResponse->setRange(minVal, maxVal);
  addSeries();
}
