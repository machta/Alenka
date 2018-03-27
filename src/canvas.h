#ifndef CANVAS_H
#define CANVAS_H

#include "SignalProcessor/lrucache.h"
#include "openglinterface.h"

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <QOpenGLWidget>

#include <cassert>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace AlenkaSignal {
class OpenCLContext;
}
class OpenDataFile;
class SignalProcessor;
class OpenGLProgram;

struct GPUCacheItem {
  GLuint signalBuffer, signalArray, eventArray;
  cl_mem sharedBuffer;
};

/**
 * @brief This class implements a GUI control for rendering signal data and
 * events.
 *
 * Every time this control gets updated the whole surface gets repainted.
 *
 * This class also handles user keyboard and mouse input. Scrolling related
 * events are skipped and handled by the parent.
 */
class Canvas : public QOpenGLWidget {
  Q_OBJECT

  OpenDataFile *file = nullptr;
  std::unique_ptr<SignalProcessor> signalProcessor;
  std::unique_ptr<OpenGLProgram> signalProgram;
  std::unique_ptr<OpenGLProgram> eventProgram;
  std::unique_ptr<OpenGLProgram> rectangleLineProgram;
  GLuint rectangleLineArray, rectangleLineBuffer;
  GLuint signalArray, signalBuffer;
  GLuint eventArray;
  bool isSelectingTrack = false;
  bool isDrawingEvent = false;
  bool isDrawingCross = false;
  int eventTrack;
  int eventStart;
  int eventEnd;
  int cursorSample = 0;
  int cursorTrack = 0;
  std::vector<QMetaObject::Connection> openFileConnections;
  std::vector<QMetaObject::Connection> montageConnections;
  std::unique_ptr<LRUCache<int, GPUCacheItem>> cache;
  int nBlock, nSamples;
  int extraSamplesFront, extraSamplesBack;
  bool duplicateSignal, glSharing;
  int parallelQueues;
  cl_command_queue commandQueue = nullptr;
  std::vector<float> processorSyncBuffer;
  std::vector<cl_mem> processorOutputBuffers;
  float sampleScale;
  bool isShiftChecked = false;
  bool isCtrlChecked = false;
  bool paintingDisabled = false;
  std::string lastGLMessage;
  int lastGLMessageCount = 0;
  bool printTiming = false;

public:
  explicit Canvas(QWidget *parent = nullptr);
  ~Canvas() override;

  /**
   * @brief Notifies this object that the DataFile changed.
   * @param file Pointer to the data file. nullptr means file was closed.
   */
  void changeFile(OpenDataFile *file);

  int getCursorPositionSample() const { return cursorSample; }
  int getCursorPositionTrack() const { return cursorTrack; }

  bool getPaintingDisabled() const { return paintingDisabled; }
  void setPaintingDisabled(bool val) { paintingDisabled = val; }

  /**
   * @brief This method defines logic of changing color of objects during
   * selection.
   *
   * This method is used at different places in code. It is defined here so that
   * this behavior is uniform.
   *
   * @todo The ad hoc method used to achieve this is not ideal and could be
   * improved.
   *
   * All three components or are modified the same way. For colors that have two
   * of the three components equal to zero or for black the resulting color
   * differs little from the original color and the selected object is not very
   * distinctive.
   */
  static QColor modifySelectionColor(const QColor &color);

  void horizontalZoom(bool reverse);
  void trackZoom(bool reverse);
  void shiftButtonCheckEvent(bool checked);
  void ctrlButtonCheckEvent(bool checked);

signals:
  void cursorPositionSampleChanged(int);
  void cursorPositionTrackChanged(int);
  void shiftZoomUp();
  void shiftZoomDown();

public slots:
  void updateCursor();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;

private:
  //! Multiply by this to convert virtual position to sample position.
  double virtualRatio();
  //! Returns the sample position of the left screen edge.
  float leftEdgePosition();
  void updateProcessor();
  void drawBlock(
      int index, GPUCacheItem *cacheItem,
      const std::vector<std::tuple<int, int, int, int>> &singleChannelEvents);
  void
  drawAllChannelEvents(const std::vector<std::tuple<int, int, int>> &events);
  void drawAllChannelEvent(int from, int to);
  void drawTimeLines();
  void drawPositionIndicator();
  void drawCross();
  // TODO: Stop using doubles for anythig connected to rendering.
  void drawTimeLine(double position);
  void drawSingleChannelEvents(
      int index, const std::vector<std::tuple<int, int, int, int>> &events);
  void drawSingleChannelEvent(int index, int track, int from, int to);
  void drawSignal(int index);
  void setUniformTrack(GLuint program, int track, int hidden, int index);
  void setUniformColor(GLuint program, const QColor &color, double opacity);
  void checkGLMessages();
  void addEvent(int channel);
  int countHiddenTracks(int track);
  void setCursorPositionSample(int sample) {
    if (sample != cursorSample) {
      cursorSample = sample;
      emit cursorPositionSampleChanged(sample);
    }
  }
  void setCursorPositionTrack(int track) {
    if (track != cursorTrack) {
      cursorTrack = track;
      emit cursorPositionTrackChanged(track);
    }
  }
  void createContext();
  void setUniformTransformMatrix(OpenGLProgram *program, float *data);
  void setUniformEventWidth(OpenGLProgram *program, float value);
  void logLastGLMessage();
  void updatePositionIndicator();

private slots:
  void updateFilter();
  void selectMontage();
  void updateMontage(int row, int col);
  void updateMontage();

  /**
   * @brief Tests whether SignalProcessor is ready to return blocks.
   *
   * This method is used to skip some code that would break if no file is
   * open and/or the current montage is empty.
   */
  bool ready();
};

#endif // CANVAS_H
