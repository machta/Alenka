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
	label->setToolTip("Enter one pairs [f, multiplier] per line. Frequencies > f are modified by multiplier (default is 1).");
	hbox->addWidget(label);
	hbox->addStretch();

	label = new QLabel("Display channel:");
	label->setToolTip("Which channel from the original file should be used?");
	hbox->addWidget(label);
	channelSlider = new QSlider(Qt::Horizontal);
	connect(channelSlider, SIGNAL(valueChanged(int)), filterVisulizer, SLOT(setChannelToDisplay(int)));
	label = new QLabel("0");
	connect(channelSlider, &QSlider::valueChanged, [label] (int value) {
		label->setText(QString::number(value));
	});
	hbox->addWidget(label);
	hbox->addWidget(channelSlider);

	box2->addLayout(hbox);
	multipliersEdit = new QPlainTextEdit();
	box2->addWidget(multipliersEdit);

	QWidget* widget = new QWidget();
	widget->setLayout(box2);
	splitter->addWidget(widget);
	box->addWidget(splitter);

	// Controls.
	hbox = new QHBoxLayout();

	hbox->addWidget(new QLabel("Filter window:"));
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
	connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(applyMultipliers()));
	hbox->addWidget(applyButton);

	hbox->addStretch();
	box->addLayout(hbox);
	setLayout(box);
}

void FilterManager::changeFile(OpenDataFile* file)
{
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
	stringstream ss(multipliersEdit->toPlainText().toStdString());

	vector<pair<double, double>> multi;
	string line;

	while (getline(ss, line), ss)
	{
		pair<double, double> p;
		double a, b;
		int got = sscanf(line.c_str(), "%lf %lf", &a, &b);

		if (1 <= got)
		{
			p.first = a;
			p.second = 2 <= got ? b : 1;
			multi.push_back(p);
		}
	}

	OpenDataFile::infoTable.setFrequencyMultipliers(multi);
}
