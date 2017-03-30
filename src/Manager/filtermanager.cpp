#include "filtermanager.h"

#include <QVBoxLayout>

using namespace std;

FilterManager::FilterManager(QWidget* parent) : QWidget(parent)
{
	filterVisulizer = new FilterVisualizer();

	QVBoxLayout* box = new QVBoxLayout();
	box->addWidget(filterVisulizer);
	setLayout(box);
}
