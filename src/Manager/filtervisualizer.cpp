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
	vector<float> absResponse(n);

	fft->fwd(response, b);

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
			filterInput(file->getFilterCoefficients());
		});
	}
	else
	{
		connection = QMetaObject::Connection();
	}
}

void FilterVisualizer::filterInput(const vector<float>& bb)
{
	if (!file)
		return;

	vector<float> response = computeFilterResponse(bb, 10000, fft);

	QLineSeries* series = new QLineSeries();
	const double fs = file->file->getSamplingFrequency()/2;
	const unsigned int n2 = response.size()/2;

	for (unsigned int i = 0; i < n2; ++i)
		series->append(fs*i/n2, response[i]);

	QChart* chart = new QChart();
	chart->addSeries(series);
	chart->createDefaultAxes();
	chart->axisX()->setRange(0, fs);
	//chart->axisY()->setRange(0, 10);
	chart->legend()->hide();

	delete chartView->chart();
	chartView->setChart(chart);
}
