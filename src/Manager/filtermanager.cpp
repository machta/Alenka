#include "filtermanager.h"

#include "../DataModel/opendatafile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

using namespace std;

FilterManager::FilterManager(QWidget* parent) : QWidget(parent)
{
	filterVisulizer = new FilterVisualizer();

	QVBoxLayout* box = new QVBoxLayout();
	box->addWidget(filterVisulizer);

	QHBoxLayout* hbox = new QHBoxLayout();
	hbox->addWidget(new QLabel("Filter window:"));

	QComboBox* windowCombo = new QComboBox();
	windowCombo->addItem("None");
	windowCombo->addItem("Hamming");
	windowCombo->addItem("Blackman");
	connect(windowCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [] (int index) {
		OpenDataFile::infoTable.setFilterWindow(static_cast<AlenkaSignal::WindowFunction>(index));
	});
	hbox->addWidget(windowCombo);

	box->addLayout(hbox);
	setLayout(box);
}
