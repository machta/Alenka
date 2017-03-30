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

	disconnect(connection);

	if (file)
	{
		connection = connect(file, &OpenDataFile::filterCoefficientsChanged, [this, file] () {
			updateChart(file->getFilterCoefficients());
		});
	}
	else
	{
		connection = QMetaObject::Connection();
	}
}

void FilterVisualizer::updateChart(const vector<float>& bb)
{
	if (!file)
		return;

	assert(channelToDisplay < file->file->getChannelCount());
	QChart* chart = new QChart();
	chart->legend()->hide();

	const double fs = file->file->getSamplingFrequency()/2;
	QValueAxis* axisX = new QValueAxis();
	axisX->setRange(0, fs);
	chart->addAxis(axisX, Qt::AlignBottom);

	// Draw signal spectrum.
	const int samplesToUse = static_cast<int>(2*file->file->getSamplingFrequency());
	const int position = static_cast<int>(OpenDataFile::infoTable.getPosition() + OpenDataFile::infoTable.getSamplesDisplayed()*OpenDataFile::infoTable.getPositionIndicator());

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
	float maxVal = 0;

	for (int i = 0; i < sampleCount/2; ++i)
	{
		float val = abs(spectrum[i]);
		maxVal = max(maxVal, val);
		signalSeries->append(fs*i/(sampleCount/2), val);
	}

	signalSeries->attachAxis(axisX);
	QValueAxis* axisY = new QValueAxis();
	axisY->setTitleText("Amplitude");
	axisY->setRange(0, maxVal);
	signalSeries->attachAxis(axisY);

	chart->addAxis(axisY, Qt::AlignLeft);
	chart->addSeries(signalSeries);

	// Draw filter response.
	vector<float> response = computeFilterResponse(bb, 10000, fft);

	QLineSeries* responseSeries = new QLineSeries();
	float minVal = 1000*1000;
	maxVal = 0;
	const unsigned int n2 = response.size()/2;

	for (unsigned int i = 0; i < n2; ++i)
	{
		float val = response[i];
		maxVal = max(maxVal, val);
		minVal = min(minVal, val);
		responseSeries->append(fs*i/n2, val);
	}

	responseSeries->attachAxis(axisX);
	axisY = new QValueAxis();
	axisY->setTitleText("Attenuation (dB)");
	axisY->setRange(minVal, maxVal);
	responseSeries->attachAxis(axisY);

	chart->addAxis(axisY, Qt::AlignRight);
	chart->addSeries(responseSeries);

	// Replace the old chart.
	delete chartView->chart();
	chartView->setChart(chart);
}
