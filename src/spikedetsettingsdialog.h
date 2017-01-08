#ifndef SPIKEDETSETTINGSDIALOG_H
#define SPIKEDETSETTINGSDIALOG_H

#include <AlenkaSignal/spikedet.h>

#include <QDialog>

class SpikedetSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SpikedetSettingsDialog(AlenkaSignal::DETECTOR_SETTINGS* settings, double* eventDuration, QWidget* parent = 0);

private:
	AlenkaSignal::DETECTOR_SETTINGS* settings;
	double* eventDuration;
};

#endif // SPIKEDETSETTINGSDIALOG_H
