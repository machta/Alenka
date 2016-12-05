#include "spikedetsettingsdialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>

using namespace std;

SpikedetSettingsDialog::SpikedetSettingsDialog(DETECTOR_SETTINGS* settings, QWidget* parent) : QDialog(parent), settings(settings)
{
	QVBoxLayout* box = new QVBoxLayout();

	QGridLayout* grid = new QGridLayout();
	box->addLayout(grid);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	box->addWidget(buttonBox);

	setLayout(box);

	QLabel* label;
	QSpinBox* spinBox;
	QDoubleSpinBox* doubleSpinBox;
	const double maxFrequency = 10000;
	const int maxDecimals = 3;

	label = new QLabel("Band low:");
	label->setToolTip("Lowpass filter frequency (-fl)");
	grid->addWidget(label, 0, 0);

	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_band_low);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_band_low = val; });
	grid->addWidget(spinBox, 0, 1);

	label = new QLabel("Band high:");
	label->setToolTip("Highpass filter frequency (-fh)");
	grid->addWidget(label, 1, 0);

	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_band_high);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_band_high = val; });
	grid->addWidget(spinBox, 1, 1);

	label = new QLabel("K1:");
	label->setToolTip("(-k1)");
	grid->addWidget(label, 2, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k1);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k1 = val; });
	grid->addWidget(doubleSpinBox, 2, 1);

	label = new QLabel("K2:");
	label->setToolTip("if K2=K1, K2 is ignored (-k2)");
	grid->addWidget(label, 3, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k2);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k2 = val; });
	grid->addWidget(doubleSpinBox, 3, 1);

	label = new QLabel("K3:");
	label->setToolTip("(-k3)");
	grid->addWidget(label, 4, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k3);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k3 = val; });
	grid->addWidget(doubleSpinBox, 4, 1);

	label = new QLabel("Winsize:");
	label->setToolTip("(-w)");
	grid->addWidget(label, 5, 0);

	spinBox = new QSpinBox();
	//spinBox->setMaximum();
	spinBox->setValue(settings->m_winsize);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_winsize = val; });
	grid->addWidget(spinBox, 5, 1);

	label = new QLabel("Noverlap:");
	label->setToolTip("(-n)");
	grid->addWidget(label, 6, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_noverlap);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_noverlap = val; });
	grid->addWidget(doubleSpinBox, 6, 1);

	label = new QLabel("Buffering:");
	label->setToolTip("(-buf)");
	grid->addWidget(label, 7, 0);

	spinBox = new QSpinBox();
	spinBox->setMaximum(10000);
	spinBox->setValue(settings->m_buffering);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_buffering = val; });
	grid->addWidget(spinBox, 7, 1);

	label = new QLabel("Main hum. freq.:");
	label->setToolTip("(-h)");
	grid->addWidget(label, 8, 0);

	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_main_hum_freq);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_main_hum_freq = val; });
	grid->addWidget(spinBox, 8, 1);

	label = new QLabel("Discharge tol.:");
	label->setToolTip("(-dt)");
	grid->addWidget(label, 9, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_discharge_tol);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_discharge_tol = val; });
	grid->addWidget(doubleSpinBox, 9, 1);

	label = new QLabel("Polyspike union time:");
	label->setToolTip("(-pt)");
	grid->addWidget(label, 10, 0);

	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_polyspike_union_time);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_polyspike_union_time = val; });
	grid->addWidget(doubleSpinBox, 10, 1);

	label = new QLabel("Decimation:");
	label->setToolTip("(-dec)");
	grid->addWidget(label, 11, 0);

	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_decimation);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_decimation = val; });
	grid->addWidget(spinBox, 11, 1);
}
