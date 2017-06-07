#ifndef TRACKLABELBAR_H
#define TRACKLABELBAR_H

#include <QColor>
#include <QString>
#include <QWidget>

#include <vector>

class OpenDataFile;

/**
 * @brief This class implements the GUI control responsible for displaying track
 * names.
 */
class TrackLabelBar : public QWidget {
  Q_OBJECT

  OpenDataFile *file = nullptr;
  int selectedTrack = -1;
  std::vector<QMetaObject::Connection> trackConnections;
  std::vector<QString> labels;
  std::vector<QColor> colors;
  std::vector<int> amplitudeArrows;
  bool drawArrows = false;

public:
  explicit TrackLabelBar(QWidget *parent = nullptr);

  /**
   * @brief Notifies this object that the DataFile changed.
   * @param file Pointer to the data file. nullptr means file was closed.
   */
  void changeFile(OpenDataFile *file) { this->file = file; }

public slots:
  void setSelectedTrack(int value) {
    selectedTrack = value;
    update();
  }

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void updateConnections(int row);
  void updateLabels();
};

#endif // TRACKLABELBAR_H
