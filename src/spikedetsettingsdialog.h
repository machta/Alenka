#ifndef SPIKEDETSETTINGSDIALOG_H
#define SPIKEDETSETTINGSDIALOG_H

#include "../../Alenka-Signal/include/AlenkaSignal/spikedet.h"

#include <QDialog>

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;

class SpikedetSettingsDialog : public QDialog {
  Q_OBJECT

  DETECTOR_SETTINGS *settings;
  double *eventDuration;
  bool *originalSpikedet;

  QSpinBox *fl_box;
  QSpinBox *fh_box;
  QDoubleSpinBox *k1_box;
  QDoubleSpinBox *k2_box;
  QDoubleSpinBox *k3_box;
  QSpinBox *w_box;
  QDoubleSpinBox *n_box;
  QSpinBox *buf_box;
  QSpinBox *h_box;
  QDoubleSpinBox *dt_box;
  QDoubleSpinBox *pt_box;
  QSpinBox *dec_box;
  QDoubleSpinBox *sed_box;

public:
  explicit SpikedetSettingsDialog(DETECTOR_SETTINGS *settings,
                                  double *eventDuration,
                                  QWidget *parent = nullptr);

  static void resetSettings(DETECTOR_SETTINGS *settings, double *eventDuration);

private:
  void setValues();
};

#endif // SPIKEDETSETTINGSDIALOG_H
