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
	void setChannelToDisplay(int i)
	{
		if (channelToDisplay != i)
		{
			channelToDisplay = i;
			updateChart();
		}
	}
	void updateChart();

private:
	OpenDataFile* file = nullptr;
	QtCharts::QChartView* chartView;
	std::vector<QMetaObject::Connection> connections;
	Eigen::FFT<float>* fft;
	std::vector<float> buffer;
	int channelToDisplay = 0;
};

#endif // FILTERVISUALIZER_H
