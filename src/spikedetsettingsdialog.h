#ifndef SPIKEDETSETTINGSDIALOG_H
#define SPIKEDETSETTINGSDIALOG_H

#include "DataFile/datafile.h"

#include <AlenkaSignal/spikedet.h>

#include <QDialog>

class SpikedetSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SpikedetSettingsDialog(AlenkaSignal::DETECTOR_SETTINGS* settings, QWidget* parent = 0);

private:
	AlenkaSignal::DETECTOR_SETTINGS* settings;
};

#endif // SPIKEDETSETTINGSDIALOG_H
