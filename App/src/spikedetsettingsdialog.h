#ifndef SPIKEDETSETTINGSDIALOG_H
#define SPIKEDETSETTINGSDIALOG_H

#include "DataFile/datafile.h"
#include "spikedet.h"

#include <QDialog>

class SpikedetSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SpikedetSettingsDialog(DETECTOR_SETTINGS* settings, QWidget* parent = 0);

private:
	DETECTOR_SETTINGS* settings;
};

#endif // SPIKEDETSETTINGSDIALOG_H
