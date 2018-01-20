#include "filtermanager.h"

#include "../DataModel/opendatafile.h"
#include "filtervisualizer.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <sstream>

using namespace std;

FilterManager::FilterManager(QWidget *parent) : QWidget(parent) {
  filterVisulizer = new FilterVisualizer();

  auto box = new QVBoxLayout();
  auto splitter = new QSplitter(Qt::Vertical);

  auto box2 = new QVBoxLayout();
  box2->setMargin(0);
  box2->addWidget(filterVisulizer);

  // Visulizer controls.
  auto hbox = new QHBoxLayout();
  hbox->addStretch();

  auto button = new QPushButton();
  button->setIcon(filterVisulizer->getResetAction()->icon());
  button->setToolTip(filterVisulizer->getResetAction()->toolTip());
  connect(button, SIGNAL(clicked(bool)), filterVisulizer->getResetAction(),
          SLOT(trigger()));
  hbox->addWidget(button);

  button = new QPushButton();
  button->setIcon(filterVisulizer->getZoomAction()->icon());
  button->setToolTip(filterVisulizer->getZoomAction()->toolTip());
  button->setCheckable(true);
  connect(button, SIGNAL(clicked(bool)), filterVisulizer->getZoomAction(),
          SLOT(setChecked(bool)));
  hbox->addWidget(button);

  QLabel *label = new QLabel("Channel");
  label->setToolTip("Index of the channel from the original file to be used as "
                    "input for the spectrum graph");
  hbox->addWidget(label);
  channelSpinBox = new QSpinBox();
  connect(channelSpinBox, SIGNAL(valueChanged(int)), filterVisulizer,
          SLOT(setChannelToDisplay(int)));
  hbox->addWidget(channelSpinBox);

  auto spinBox = new QSpinBox();
  spinBox->setRange(0, 100);
  connect(spinBox, SIGNAL(valueChanged(int)), filterVisulizer,
          SLOT(setSecondsToDisplay(int)));
  spinBox->setValue(2);
  hbox->addWidget(spinBox);
  label = new QLabel("sec");
  label->setToolTip("Interval in seconds to be used for the spectrum graph");
  hbox->addWidget(label);

  QCheckBox *checkBox = new QCheckBox("freeze");
  checkBox->setToolTip("Freeze spectrum graph");
  connect(checkBox, SIGNAL(clicked(bool)), filterVisulizer,
          SLOT(setFreezeSpectrum(bool)));
  checkBox->setChecked(true);
  hbox->addWidget(checkBox);

  box2->addLayout(hbox);
  box2->addSpacing(5);
  auto widget = new QWidget();
  widget->setLayout(box2);
  splitter->addWidget(widget);

  // Multipliers text field.
  box2 = new QVBoxLayout();
  box2->setMargin(0);
  box2->addSpacing(5);

  label = new QLabel("Frequency multipliers:");
  label->setToolTip("Enter one pair of numbers per line [f, multiplier]; "
                    "frequencies > f are modified by the multiplier");
  box2->addWidget(label);

  multipliersEdit = new QPlainTextEdit();
  connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersChanged()), this,
          SLOT(setMultipliersText()));
  box2->addWidget(multipliersEdit);

  widget = new QWidget();
  widget->setLayout(box2);
  splitter->addWidget(widget);
  box->addWidget(splitter);

  // Filter controls.
  hbox = new QHBoxLayout();

  label = new QLabel("Filter window:");
  label->setToolTip(
      "Window function to be used to modify the FIR filter coeficients");
  hbox->addWidget(label);
  auto windowCombo = new QComboBox();
  windowCombo->addItem("None");
  windowCombo->addItem("Hamming");
  windowCombo->addItem("Blackman");
  connect(
      windowCombo,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [](int index) {
        OpenDataFile::infoTable.setFilterWindow(
            static_cast<AlenkaSignal::WindowFunction>(index));
      });
  connect(&OpenDataFile::infoTable, &InfoTable::filterWindowChanged,
          [windowCombo](AlenkaSignal::WindowFunction index) {
            windowCombo->setCurrentIndex(static_cast<int>(index));
          });
  hbox->addWidget(windowCombo);

  hbox->addStretch();

  QPushButton *applyButton = new QPushButton("Apply");
  applyButton->setToolTip("Apply multipliers");
  connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(applyMultipliers()));
  hbox->addWidget(applyButton);

  checkBox = new QCheckBox("on");
  checkBox->setToolTip("Enable/disable multipliers");
  connect(checkBox, SIGNAL(clicked(bool)), &OpenDataFile::infoTable,
          SLOT(setFrequencyMultipliersOn(bool)));
  connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersOnChanged(bool)),
          checkBox, SLOT(setChecked(bool)));
  hbox->addWidget(checkBox);

  box->addLayout(hbox);
  setLayout(box);
}

void FilterManager::changeFile(OpenDataFile *file) {
  this->file = file;
  filterVisulizer->changeFile(file);

  if (file) {
    channelSpinBox->setRange(0, file->file->getChannelCount() - 1);
    channelSpinBox->setValue(0);
  }
}

void FilterManager::setMultipliersText() {
  QString str;

  auto value = OpenDataFile::infoTable.getFrequencyMultipliers();

  for (auto e : value)
    str += QString::number(e.first) + ' ' + QString::number(e.second) + '\n';

  multipliersEdit->setPlainText(str);
}

void FilterManager::applyMultipliers() {
  if (!file)
    return;

  stringstream ss(multipliersEdit->toPlainText().toStdString());

  const double fs = file->file->getSamplingFrequency() / 2;
  vector<pair<double, double>> multi;
  string line;

  while (getline(ss, line), ss) {
    pair<double, double> p;
    double a, b;
    int got = sscanf(line.c_str(), "%lf %lf", &a, &b);

    if (1 <= got && a <= fs) {
      p.first = a;
      p.second = (2 == got ? b : 1);
      multi.push_back(p);
    }
  }

  OpenDataFile::infoTable.setFrequencyMultipliers(multi);
}
