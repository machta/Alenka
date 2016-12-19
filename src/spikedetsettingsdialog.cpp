#include "spikedetsettingsdialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>

using namespace std;

SpikedetSettingsDialog::SpikedetSettingsDialog(AlenkaSignal::DETECTOR_SETTINGS* settings, QWidget* parent) : QDialog(parent), settings(settings)
{
	QVBoxLayout* box = new QVBoxLayout();

	QFormLayout* grid = new QFormLayout();
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
	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_band_low);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_band_low = val; });
	grid->addRow(label, spinBox);

	label = new QLabel("Band high:");
	label->setToolTip("Highpass filter frequency (-fh)");
	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_band_high);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_band_high = val; });
	grid->addRow(label, spinBox);

	label = new QLabel("K1:");
	label->setToolTip("(-k1)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k1);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k1 = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("K2:");
	label->setToolTip("if K2=K1, K2 is ignored (-k2)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k2);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k2 = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("K3:");
	label->setToolTip("(-k3)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	doubleSpinBox->setMaximum(1000);
	doubleSpinBox->setValue(settings->m_k3);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_k3 = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("Winsize:");
	label->setToolTip("(-w)");
	spinBox = new QSpinBox();
	//spinBox->setMaximum();
	spinBox->setValue(settings->m_winsize);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_winsize = val; });
	grid->addRow(label, spinBox);

	label = new QLabel("Noverlap:");
	label->setToolTip("(-n)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_noverlap);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_noverlap = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("Buffering:");
	label->setToolTip("(-buf)");
	spinBox = new QSpinBox();
	spinBox->setMaximum(10000);
	spinBox->setValue(settings->m_buffering);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_buffering = val; });
	grid->addRow(label, spinBox);

	label = new QLabel("Main hum. freq.:");
	label->setToolTip("(-h)");
	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_main_hum_freq);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_main_hum_freq = val; });
	grid->addRow(label, spinBox);

	label = new QLabel("Discharge tol.:");
	label->setToolTip("(-dt)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_discharge_tol);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_discharge_tol = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("Polyspike union time:");
	label->setToolTip("(-pt)");
	doubleSpinBox = new QDoubleSpinBox();
	doubleSpinBox->setDecimals(maxDecimals);
	//doubleSpinBox->setMaximum();
	doubleSpinBox->setValue(settings->m_polyspike_union_time);
	connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [settings] (double val) { settings->m_polyspike_union_time = val; });
	grid->addRow(label, doubleSpinBox);

	label = new QLabel("Decimation:");
	label->setToolTip("(-dec)");
	spinBox = new QSpinBox();
	spinBox->setMaximum(maxFrequency);
	spinBox->setValue(settings->m_decimation);
	connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [settings] (int val) { settings->m_decimation = val; });
	grid->addRow(label, spinBox);
}
