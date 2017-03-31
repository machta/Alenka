#include "filtervisualizer.h"

#include "../DataModel/opendatafile.h"

#include <QtCharts>
#include <QVBoxLayout>

#include <cmath>
#include <complex>

using namespace std;
using namespace QtCharts;

namespace
{

vector<float> computeFilterResponse(const vector<float>& bb, const unsigned int n, Eigen::FFT<float>* fft)
{
	assert(bb.size() <= n);

	vector<float> b = bb;
	b.resize(n);

	vector<complex<float>> response;
	fft->fwd(response, b);

	vector<float> absResponse(n);
	for (unsigned int i = 0; i < n; ++i)
		absResponse[i] = 20*log10(abs(response[i]));

	return absResponse;
}

} // namespace

FilterVisualizer::FilterVisualizer(QWidget* parent) : QWidget(parent)
{
	fft = new Eigen::FFT<float>();

	chartView = new QChartView();
	chartView->setRenderHint(QPainter::Antialiasing);

	QVBoxLayout* box = new QVBoxLayout();
	box->addWidget(chartView);
	setLayout(box);
}

void FilterVisualizer::changeFile(OpenDataFile* file)
{
	this->file = file;

	for (auto e : connections)
		disconnect(e);
	connections.clear();

	if (file)
	{
		auto c = connect(file, SIGNAL(filterCoefficientsChanged()), this, SLOT(updateChart()));
		connections.push_back(c);

		c = connect(&file->infoTable, SIGNAL(positionChanged(int)), this, SLOT(updateChart()));
		connections.push_back(c);

		c = connect(&file->infoTable, SIGNAL(positionIndicatorChanged(double)), this, SLOT(updateChart()));
		connections.push_back(c);

		c = connect(&file->infoTable, SIGNAL(pixelViewWidthChanged(int)), SLOT(updateChart()));
		connections.push_back(c);
	}
}

void FilterVisualizer::updateChart()
{
	if (!file)
		return;

	assert(channelToDisplay < static_cast<int>(file->file->getChannelCount()));
	QChart* chart = new QChart();
	chart->legend()->hide();

	const double fs = file->file->getSamplingFrequency()/2;
	QValueAxis* axisX = new QValueAxis();
	axisX->setRange(0, fs);
	chart->addAxis(axisX, Qt::AlignBottom);

	// Draw signal spectrum.
	const int samplesToUse = 2*static_cast<int>(file->file->getSamplingFrequency());
	double ratio = static_cast<double>(file->file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
	double doublePosition = OpenDataFile::infoTable.getPosition() + OpenDataFile::infoTable.getPixelViewWidth()*OpenDataFile::infoTable.getPositionIndicator();
	const int position = static_cast<int>(doublePosition*ratio);

	const int start = max(0, position - samplesToUse/2);
	const int end = start + samplesToUse - 1;
	const int sampleCount = end - start + 1; // TODO: Round this to a power of two.

	buffer.resize(sampleCount*file->file->getChannelCount());
	file->file->readSignal(buffer.data(), start, end);

	auto begin = buffer.begin() + channelToDisplay*sampleCount;
	vector<float> input(begin, begin + sampleCount);

	vector<complex<float>> spectrum;
	fft->fwd(spectrum, input);

	assert(static_cast<int>(input.size()) == sampleCount);
	assert(static_cast<int>(spectrum.size()) == sampleCount);

	QLineSeries* signalSeries = new QLineSeries();
	chart->addSeries(signalSeries);
	signalSeries->attachAxis(axisX);

	QValueAxis* axisY = new QValueAxis();
	chart->addAxis(axisY, Qt::AlignLeft);
	signalSeries->attachAxis(axisY);
	axisY->setTitleText("Amplitude");

	float maxVal = 0;

	for (int i = 0; i < sampleCount/2; ++i)
	{
		float val = abs(spectrum[i]);
		if (isfinite(val))
		{
			maxVal = max(maxVal, val);
			signalSeries->append(fs*i/(sampleCount/2), val);
		}
	}

	axisY->setRange(0, maxVal);

	// Draw filter response.
	vector<float> response = computeFilterResponse(file->getFilterCoefficients(), 10000, fft);

	QLineSeries* responseSeries = new QLineSeries();
	chart->addSeries(responseSeries);
	responseSeries->attachAxis(axisX);

	axisY = new QValueAxis();
	chart->addAxis(axisY, Qt::AlignRight);
	responseSeries->attachAxis(axisY);
	axisY->setTitleText("Attenuation (dB)");

	float minVal = 1000*1000;
	maxVal = 0;
	const unsigned int n2 = response.size()/2;

	for (unsigned int i = 0; i < n2; ++i)
	{
		float val = response[i];
		if (isfinite(val))
		{
			maxVal = max(maxVal, val);
			minVal = min(minVal, val);
			responseSeries->append(fs*i/n2, val);
		}
	}

	axisY->setRange(minVal, maxVal);

	// Replace the old chart.
	//delete chartView->chart();
	chartView->setChart(chart);
}
