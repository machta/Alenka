#include "filtermanager.h"

#include "../DataModel/opendatafile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QAction>

#include <sstream>

using namespace std;

FilterManager::FilterManager(QWidget* parent) : QWidget(parent)
{
	filterVisulizer = new FilterVisualizer();

	QSplitter* splitter = new QSplitter(Qt::Vertical);

	QVBoxLayout* box = new QVBoxLayout();
	splitter->addWidget(filterVisulizer);

	QVBoxLayout* box2 = new QVBoxLayout();
	QHBoxLayout* hbox = new QHBoxLayout();

	QLabel* label = new QLabel("Frequency multipliers:");
	label->setToolTip("Enter one pairs [f, multiplier] per line. Frequencies > f are modified by the multiplier");
	hbox->addWidget(label);

	QCheckBox* checkBox = new QCheckBox("on");
	checkBox->setToolTip("Enable/disable multipliers");
	connect(checkBox, SIGNAL(clicked(bool)), &OpenDataFile::infoTable, SLOT(setFrequencyMultipliersOn(bool)));
	connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersOnChanged(bool)), checkBox, SLOT(setChecked(bool)));
	hbox->addWidget(checkBox);

	hbox->addStretch();

	// Visulizer controls.
	QPushButton* button = new QPushButton();
	button->setIcon(filterVisulizer->getResetAction()->icon());
	button->setToolTip(filterVisulizer->getResetAction()->toolTip());
	connect(button, SIGNAL(clicked(bool)), filterVisulizer->getResetAction(), SLOT(trigger()));
	hbox->addWidget(button);

	button = new QPushButton();
	button->setIcon(filterVisulizer->getZoomAction()->icon());
	button->setToolTip(filterVisulizer->getZoomAction()->toolTip());
	button->setCheckable(true);
	connect(button, SIGNAL(clicked(bool)), filterVisulizer->getZoomAction(), SLOT(setChecked(bool)));
	hbox->addWidget(button);

	label = new QLabel("Channel");
	label->setToolTip("Index of a channel from the original file to use as input for the spectrum graph");
	hbox->addWidget(label);
	channelSlider = new QSlider(Qt::Horizontal);
	connect(channelSlider, SIGNAL(valueChanged(int)), filterVisulizer, SLOT(setChannelToDisplay(int)));
	label = new QLabel("0");
	connect(channelSlider, &QSlider::valueChanged, [label] (int value) {
		label->setText(QString::number(value));
	});
	hbox->addWidget(label);
	hbox->addWidget(channelSlider);

	QSpinBox* spinBox = new QSpinBox();
	spinBox->setRange(0, 100);
	connect(spinBox, SIGNAL(valueChanged(int)), filterVisulizer, SLOT(setSecondsToDisplay(int)));
	spinBox->setValue(2);
	hbox->addWidget(spinBox);
	label = new QLabel("s");
	label->setToolTip("Interval in seconds to use for the spectrum graph");
	hbox->addWidget(label);

	checkBox = new QCheckBox("freeze");
	checkBox->setToolTip("Freeze spectrum");
	connect(checkBox, SIGNAL(clicked(bool)), filterVisulizer, SLOT(setFreezeSpectrum(bool)));
	checkBox->setChecked(false);
	hbox->addWidget(checkBox);

	box2->addLayout(hbox);

	// Multipliers text field.
	multipliersEdit = new QPlainTextEdit();
	connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersChanged(std::vector<std::pair<double,double> >)), this, SLOT(setMultipliersText(std::vector<std::pair<double,double> >)));
	box2->addWidget(multipliersEdit);

	QWidget* widget = new QWidget();
	widget->setLayout(box2);
	splitter->addWidget(widget);
	box->addWidget(splitter);

	// Filter controls.
	hbox = new QHBoxLayout();

	label = new QLabel("Filter window:");
	label->setToolTip("Window function to use to modify the FIR filter coeficients");
	hbox->addWidget(label);
	QComboBox* windowCombo = new QComboBox();
	windowCombo->addItem("None");
	windowCombo->addItem("Hamming");
	windowCombo->addItem("Blackman");
	connect(windowCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [] (int index) {
		OpenDataFile::infoTable.setFilterWindow(static_cast<AlenkaSignal::WindowFunction>(index));
	});
	connect(&OpenDataFile::infoTable, &InfoTable::filterWindowChanged, [windowCombo] (AlenkaSignal::WindowFunction index) {
		windowCombo->setCurrentIndex(static_cast<int>(index));
	});
	hbox->addWidget(windowCombo);

	QPushButton* applyButton = new QPushButton("Apply");
	applyButton->setToolTip("Apply multipliers");
	connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(applyMultipliers()));
	hbox->addWidget(applyButton);

	hbox->addStretch();
	box->addLayout(hbox);
	setLayout(box);
}

void FilterManager::changeFile(OpenDataFile* file)
{
	this->file = file;
	filterVisulizer->changeFile(file);

	if (file)
	{
		channelSlider->setRange(0, file->file->getChannelCount() - 1);
		channelSlider->setValue(0);
	}
}

void FilterManager::setMultipliersText(const std::vector<std::pair<double, double>>& value)
{
	QString str;

	for (auto e : value)
		str += QString::number(e.first) + ' ' + QString::number(e.second) + '\n';

	multipliersEdit->setPlainText(str);
}

void FilterManager::applyMultipliers()
{
	if (!file)
		return;

	stringstream ss(multipliersEdit->toPlainText().toStdString());

	const double fs = file->file->getSamplingFrequency()/2;
	vector<pair<double, double>> multi;
	string line;

	while (getline(ss, line), ss)
	{
		pair<double, double> p;
		double a, b;
		int got = sscanf(line.c_str(), "%lf %lf", &a, &b);

		if (1 <= got && a <= fs)
		{
			p.first = a;
			p.second = (2 == got ? b : 1);
			multi.push_back(p);
		}
	}

	OpenDataFile::infoTable.setFrequencyMultipliers(multi);
}
