#ifndef FILTERVISUALIZER_H
#define FILTERVISUALIZER_H

#include <QWidget>

#include <unsupported/Eigen/FFT>

#include <vector>

class OpenDataFile;
namespace QtCharts
{
class QChartView;
}

class FilterVisualizer : public QWidget
{
	Q_OBJECT

public:
	explicit FilterVisualizer(QWidget* parent = nullptr);
	~FilterVisualizer()
	{
		delete fft;
	}

	void changeFile(OpenDataFile* file);

public slots:
	void setChannelToDisplay(unsigned int i)
	{
		channelToDisplay = i;
	}

private:
	OpenDataFile* file;
	QtCharts::QChartView* chartView;
	QMetaObject::Connection connection;
	Eigen::FFT<float>* fft;
	std::vector<float> buffer;
	unsigned int channelToDisplay = 0;

	void updateChart(const std::vector<float>& b);
};

#endif // FILTERVISUALIZER_H
