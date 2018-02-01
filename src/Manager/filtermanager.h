#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include <QWidget>

class FilterVisualizer;
class OpenDataFile;
class QPlainTextEdit;
class QSpinBox;

/**
 * @brief This class implements the window for managing custom filtering.
 */
class FilterManager : public QWidget {
  Q_OBJECT

  OpenDataFile *file;
  FilterVisualizer *filterVisulizer;
  QPlainTextEdit *multipliersEdit;
  QSpinBox *channelSpinBox;

public:
  explicit FilterManager(QWidget *parent = nullptr);

  void changeFile(OpenDataFile *file);

private slots:
  void setMultipliersText();
  void applyMultipliers();
};

#endif // FILTERMANAGER_H
