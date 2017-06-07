#ifndef SIGNALVIEWER_H
#define SIGNALVIEWER_H

#include <QWidget>

#include <vector>

class OpenDataFile;
class TrackLabelBar;
class QScrollBar;
class QSplitter;
class Canvas;

/**
 * @brief This class implements the GUI control for browsing the DataFile's
 * signal.
 *
 * This control combines in itself a NewCanvas, a QScrollBar, and a
 * TrackLabelBar.
 * It constructs them, connects the appropriate signals and slots and
 * dispatches events between NewCanvas and the QScrollBar.
 *
 * Also resizing and handling of virtual width and position is done here.
 */
class SignalViewer : public QWidget {
  Q_OBJECT

public:
  explicit SignalViewer(QWidget *parent = nullptr);
  ~SignalViewer() override;

  Canvas *getCanvas() { return canvas; }

  /**
   * @brief Notifies this object that the DataFile changed.
   * @param file Pointer to the data file. nullptr means file was closed.
   */
  void changeFile(OpenDataFile *file);

signals:
  void virtualWidthChanged(int);
  void positionChanged(int);

public slots:
  void updateSignalViewer();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private:
  OpenDataFile *file = nullptr;
  QSplitter *splitter;
  Canvas *canvas;
  QScrollBar *scrollBar;
  TrackLabelBar *trackLabelBar;
  std::vector<QMetaObject::Connection> openFileConnections;

  void resize(int virtualWidth);
  int virtualWidthFromScrollBar();

private slots:
  void setVirtualWidth(int value);
  void setPosition(int value);
};

#endif // SIGNALVIEWER_H
