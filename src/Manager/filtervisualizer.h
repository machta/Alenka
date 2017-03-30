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

private:
	OpenDataFile* file;
	QtCharts::QChartView* chartView;
	QMetaObject::Connection connection;
	Eigen::FFT<float>* fft;

	void filterInput(const std::vector<float>& b);
};

#endif // FILTERVISUALIZER_H
