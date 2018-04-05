#include "canvas.h"

#include "../Alenka-File/include/AlenkaFile/datafile.h"
#include "DataModel/opendatafile.h"
#include "DataModel/undocommandfactory.h"
#include "DataModel/vitnessdatamodel.h"
#include "SignalProcessor/signalprocessor.h"
#include "error.h"
#include "myapplication.h"
#include "openglprogram.h"
#include "options.h"
#include "signalviewer.h"

#include <QCursor>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QOpenGLDebugLogger>
#include <QUndoCommand>
#include <QWheelEvent>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>

#include <detailedexception.h>

#ifndef __APPLE__
#if defined WIN_BUILD
#include <windows.h>
#elif defined UNIX_BUILD
#include <GL/glx.h>
#endif
#endif

using namespace std;
using namespace AlenkaFile;
using namespace AlenkaSignal;

namespace {

const double HORIZONTAL_ZOOM = 1.3;
const double VERTICAL_ZOOM = 1.3;
const double TRACK_ZOOM = VERTICAL_ZOOM;

void getEventTypeColorOpacity(OpenDataFile *file, int type, QColor *color,
                              double *opacity) {
  assert(color && opacity);
  EventType et = file->dataModel->eventTypeTable()->row(type);
  *color = DataModel::array2color<QColor>(et.color);
  *opacity = et.opacity;
}

const AbstractTrackTable *getTrackTable(OpenDataFile *file) {
  return file->dataModel->montageTable()->trackTable(
      OpenDataFile::infoTable.getSelectedMontage());
}

const AbstractEventTable *getEventTable(OpenDataFile *file) {
  return file->dataModel->montageTable()->eventTable(
      OpenDataFile::infoTable.getSelectedMontage());
}

class ZoomCommand : public QUndoCommand {
  DataModel *dataModel;
  int i, j;
  double before, after;
  const int commandId;
  vector<unique_ptr<QUndoCommand>> childCommands;

public:
  ZoomCommand(DataModel *dataModel, int i, int j, double before, double after)
      : QUndoCommand(), dataModel(dataModel), i(i), j(j), before(before),
        after(after), commandId(i * 1000 * 1000 + j) {
    Track t = dataModel->montageTable()->trackTable(i)->row(j);
    string text = "zoom " + t.label;
    setText(QString::fromStdString(text));
  }

  void redo() override {
    Track t = dataModel->montageTable()->trackTable(i)->row(j);
    t.amplitude = after;
    dataModel->montageTable()->trackTable(i)->row(j, t);

    for (auto &c : childCommands)
      c->redo();
  }
  void undo() override {
    for (auto i = childCommands.rbegin(); i != childCommands.rend(); ++i)
      (*i)->undo();

    Track t = dataModel->montageTable()->trackTable(i)->row(j);
    t.amplitude = before;
    dataModel->montageTable()->trackTable(i)->row(j, t);
  }

  int id() const override { return commandId; }
  bool mergeWith(const QUndoCommand *other) override {
    assert(other->id() == commandId);

    auto o = dynamic_cast<const ZoomCommand *>(other);
    childCommands.push_back(make_unique<ZoomCommand>(o->dataModel, o->i, o->j,
                                                     o->before, o->after));

    return true;
  }
};

void zoom(OpenDataFile *file, double factor, int i) {
  Track track = getTrackTable(file)->row(i);

  const double before = track.amplitude;

  double after = before * factor;
  after = after != 0 ? after : -0.000001;

  file->undoFactory->push(new ZoomCommand(
      file->file->getDataModel(), OpenDataFile::infoTable.getSelectedMontage(),
      i, before, after));
}

/**
 * @brief A convenience function for resolving blocks needed to cover a sample
 * range.
 * @param range The sample range.
 * @param blockSize The size of the blocks constant for all blocks.
 */
pair<int64_t, int64_t> sampleRangeToBlockRange(int64_t from, int64_t to,
                                               unsigned int blockSize) {
  from /= blockSize - 1;
  to = (to - 1) / (blockSize - 1);

  return make_pair(from, to);
}

void getEventsForRendering(
    OpenDataFile *file, int firstSample, int lastSample,
    vector<tuple<int, int, int>> *allChannelEvents,
    vector<tuple<int, int, int, int>> *singleChannelEvents) {
  const AbstractEventTable *eventTable = getEventTable(file);

  for (int i = 0; i < eventTable->rowCount(); ++i) {
    Event e = eventTable->row(i);

    if (e.position <= lastSample &&
        firstSample <= e.position + e.duration - 1 && e.type >= 0 &&
        e.channel >= -1 &&
        file->dataModel->eventTypeTable()->row(e.type).hidden == false) {
      if (e.channel == -1) {
        allChannelEvents->emplace_back(e.type, e.position, e.duration);
      } else {
        if (getTrackTable(file)->row(e.channel).hidden == false)
          singleChannelEvents->emplace_back(e.type, e.channel, e.position,
                                            e.duration);
      }
    }
  }

  sort(allChannelEvents->begin(), allChannelEvents->end(),
       [](tuple<int, int, int> a, tuple<int, int, int> b) {
         return get<0>(a) < get<0>(b);
       });

  stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(),
              [](tuple<int, int, int, int> a, tuple<int, int, int, int> b) {
                return get<1>(a) < get<1>(b);
              });
  stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(),
              [](tuple<int, int, int, int> a, tuple<int, int, int, int> b) {
                return get<0>(a) < get<0>(b);
              });
  // TODO: Use array<> instead of a tupple<>.
}

void arrayAttrib(GLint size, GLsizei stride) {
  gl()->glVertexAttribPointer(0, size, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void *>(0));
  gl()->glEnableVertexAttribArray(0);
}

void bindArray(GLuint array, GLuint buffer, GLint size, GLsizei stride) {
  if (programOption<bool>("gl20")) {
    gl()->glBindBuffer(GL_ARRAY_BUFFER, buffer);
    arrayAttrib(size, stride);
  } else {
    gl3()->glBindVertexArray(array);
  }
}

class GPUCacheAllocator : public LRUCacheAllocator<GPUCacheItem> {
  size_t size;
  bool duplicateSignal, glSharing;
  OpenCLContext *context;

public:
  GPUCacheAllocator(size_t size, bool duplicateSignal, bool glSharing,
                    OpenCLContext *context)
      : size(size), duplicateSignal(duplicateSignal), glSharing(glSharing),
        context(context) {}

  bool constructElement(GPUCacheItem **ptr) override {
    *ptr = new GPUCacheItem();
    bool ret = initializeElement(**ptr);

    if (!ret) {
      logToFileAndConsole(
          "Memory allocation failed. The GPU cache will not grow anymore.");
    }

    return ret;
  }
  void destroyElement(GPUCacheItem *ptr) override {
    if (ptr) {
      if (!programOption<bool>("gl20")) {
        if (ptr->signalArray)
          gl3()->glDeleteVertexArrays(1, &ptr->signalArray);

        if (ptr->eventArray)
          gl3()->glDeleteVertexArrays(1, &ptr->eventArray);
      }

      if (ptr->sharedBuffer) {
        cl_int err = clReleaseMemObject(ptr->sharedBuffer);
        checkClErrorCode(err, "clReleaseMemObject()");
      }

      if (ptr->signalBuffer)
        gl()->glDeleteBuffers(1, &ptr->signalBuffer);

      delete ptr;
    }
  }

private:
  bool initializeElement(GPUCacheItem &item) {
    item.signalBuffer = item.signalArray = item.eventArray = 0;
    item.sharedBuffer = nullptr;

    gl()->glGenBuffers(1, &item.signalBuffer);
    gl()->glBindBuffer(GL_ARRAY_BUFFER, item.signalBuffer);
    if (glSharing)
      gl()->glBufferData(GL_ARRAY_BUFFER, size, nullptr,
                         /*GL_STATIC_DRAW*/ GL_DYNAMIC_DRAW);

    if (OPENGL_INTERFACE->checkGLErrors())
      return false; // Out of graphical memory.

    if (!programOption<bool>("gl20")) {
      GLuint arrays[2];
      gl3()->glGenVertexArrays(2, arrays);
      item.signalArray = arrays[0];
      item.eventArray = arrays[1];

      gl3()->glBindVertexArray(item.signalArray);
    }
    arrayAttrib(1, duplicateSignal ? 2 * sizeof(float) : 0);

    if (!programOption<bool>("gl20"))
      gl3()->glBindVertexArray(item.eventArray);

    if (duplicateSignal) {
      arrayAttrib(1, 0);
    } else { // TODO: Add this case to arrayAttrib;
      gl4()->glBindVertexBuffer(0, item.signalBuffer, 0, 0);
      gl4()->glVertexBindingDivisor(0, 2);

      gl4()->glVertexAttribFormat(0, 1, GL_FLOAT, GL_FALSE, 0);
      gl4()->glVertexAttribBinding(0, 0);
      gl()->glEnableVertexAttribArray(0);
    }

    gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (glSharing) {
      cl_int err;
      cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
      flags = CL_MEM_WRITE_ONLY;
#endif
      item.sharedBuffer = clCreateFromGLBuffer(context->getCLContext(), flags,
                                               item.signalBuffer, &err);

      if (err == CL_OUT_OF_HOST_MEMORY) {
        return false;
      } else {
        checkClErrorCode(err, "clCreateFromGLBuffer()");
      }
    } else {
      item.sharedBuffer = nullptr;
    }

    return true;
  }
};

} // namespace

Canvas::Canvas(QWidget *parent) : QOpenGLWidget(parent) {
  setFocusPolicy(Qt::ClickFocus);
  setMouseTracking(true);

  // TODO: Fix the OpenGL 4.3 optimization.
  programOption("parProc", parallelQueues);
  programOption("blockSize", nBlock);
  duplicateSignal = !programOption<bool>("gl43");
  programOption("glSharing", glSharing);
  printTiming = isProgramOptionSet("printTiming");

  extraSamplesFront = extraSamplesBack = 0; // TODO: Test this with other values
                                            // if it is ever needed for for the
                                            // "pretty" event rendering;
}

Canvas::~Canvas() {
  if (programOption<bool>("kernelCachePersist"))
    OpenDataFile::kernelCache->saveToFile();

  makeCurrent();

  if (!programOption<bool>("gl20"))
    gl3()->glDeleteVertexArrays(1, &rectangleLineArray);
  gl()->glDeleteBuffers(1, &rectangleLineBuffer);

  cl_int err;
  for (cl_mem e : processorOutputBuffers) {
    err = clReleaseMemObject(e);
    checkClErrorCode(err, "clReleaseMemObject()");
  }

  if (commandQueue) {
    err = clReleaseCommandQueue(commandQueue);
    checkClErrorCode(err, "clReleaseCommandQueue()");
  }

  // Release these three objects here explicitly to make sure the right GL
  // context is bound by makeCurrent().
  signalProgram.reset();
  eventProgram.reset();
  rectangleLineProgram.reset();

  logLastGLMessage();
  OPENGL_INTERFACE->checkGLErrors();
  doneCurrent();
}

void Canvas::changeFile(OpenDataFile *file) {
  makeCurrent();

  this->file = file;

  if (file) {
    auto c = connect(&OpenDataFile::infoTable,
                     SIGNAL(lowpassFrequencyChanged(double)), this,
                     SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassOnChanged(bool)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable,
                SIGNAL(highpassFrequencyChanged(double)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable, SIGNAL(highpassOnChanged(bool)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable, SIGNAL(notchOnChanged(bool)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable,
                SIGNAL(filterWindowChanged(AlenkaSignal::WindowFunction)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable, SIGNAL(frequencyMultipliersChanged()),
                this, SLOT(updateFilter()));
    openFileConnections.push_back(c);
    c = connect(&OpenDataFile::infoTable,
                SIGNAL(frequencyMultipliersOnChanged(bool)), this,
                SLOT(updateFilter()));
    openFileConnections.push_back(c);

    c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)),
                this, SLOT(selectMontage()));
    openFileConnections.push_back(c);

    c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)),
                this, SLOT(updateCursor()));
    openFileConnections.push_back(c);

    function<void()> sharingFunction = nullptr;
    if (glSharing)
      sharingFunction = [this]() { gl()->glFinish(); };

    signalProcessor = make_unique<SignalProcessor>(
        nBlock, parallelQueues, duplicateSignal ? 2 : 1, sharingFunction, file,
        globalContext.get(), extraSamplesFront, extraSamplesBack);
  } else {
    for (auto e : openFileConnections)
      disconnect(e);
    openFileConnections.clear();

    signalProcessor.reset(nullptr);
  }

  doneCurrent();
}

QColor Canvas::modifySelectionColor(const QColor &color) {
  double colorComponents[3] = {color.redF(), color.greenF(), color.blueF()};

  for (double &colorComponent : colorComponents)
    colorComponent += colorComponent > 0.5 ? -0.45 : 0.45;

  QColor newColor(color);
  newColor.setRedF(colorComponents[0]);
  newColor.setGreenF(colorComponents[1]);
  newColor.setBlueF(colorComponents[2]);
  return newColor;
}

void Canvas::horizontalZoom(bool reverse) {
  double factor = HORIZONTAL_ZOOM;
  if (reverse)
    factor = 1 / factor;

  OpenDataFile::infoTable.setVirtualWidth(
      OpenDataFile::infoTable.getVirtualWidth() * factor);
}

void Canvas::trackZoom(bool reverse) {
  if (file) {
    double factor = TRACK_ZOOM;
    if (reverse)
      factor = 1 / factor;

    int track = cursorTrack;
    track += countHiddenTracks(track);

    zoom(file, factor, track);
  }
}

void Canvas::shiftButtonCheckEvent(bool checked) { isShiftChecked = checked; }

void Canvas::ctrlButtonCheckEvent(bool checked) { isCtrlChecked = checked; }

void Canvas::updateCursor() {
  if (ready()) {
    const QPoint pos = mapFromGlobal(QCursor::pos());
    const int sample = round(pos.x() * virtualRatio() + leftEdgePosition());

    const double trackHeigth =
        static_cast<double>(height()) / signalProcessor->getTrackCount();
    const int track = static_cast<int>(pos.y() / trackHeigth);

    setCursorPositionSample(sample);
    setCursorPositionTrack(track);

    if (isDrawingEvent) {
      eventEnd = sample;
      update();
    }
  }
}

void Canvas::initializeGL() {
  logToFile("Initializing OpenGL in Canvas.");

  OPENGL_INTERFACE = make_unique<OpenGLInterface>();
  OPENGL_INTERFACE->initializeOpenGLInterface();

  QFile signalVertFile(":/signal.vert");
  signalVertFile.open(QIODevice::ReadOnly);
  string signalVert = signalVertFile.readAll().toStdString();

  QFile eventVertFile(":/event.vert");
  eventVertFile.open(QIODevice::ReadOnly);
  string eventVert = eventVertFile.readAll().toStdString();

  QFile rectangleLineVertFile(":/rectangleLine.vert");
  rectangleLineVertFile.open(QIODevice::ReadOnly);
  string rectangleLineVert = rectangleLineVertFile.readAll().toStdString();

  QFile colorFragFile(":/color.frag");
  colorFragFile.open(QIODevice::ReadOnly);
  string colorFrag = colorFragFile.readAll().toStdString();

  signalProgram = make_unique<OpenGLProgram>(signalVert, colorFrag);
  eventProgram = make_unique<OpenGLProgram>(eventVert, colorFrag);
  rectangleLineProgram =
      make_unique<OpenGLProgram>(rectangleLineVert, colorFrag);

  gl()->glEnable(GL_BLEND);
  gl()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl()->glClearColor(1, 1, 1, 0);

  gl()->glGenBuffers(1, &rectangleLineBuffer);
  gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

  if (!programOption<bool>("gl20")) {
    gl3()->glGenVertexArrays(1, &rectangleLineArray);
    gl3()->glBindVertexArray(rectangleLineArray);
  }
  arrayAttrib(2, 0);

  gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
  //	gl3()->glBindVertexArray(0);

  // Log OpenGL details.
  stringstream ss;

  ss << "OpenGL info:" << endl;
  ss << "Version: " << gl()->glGetString(GL_VERSION) << endl;
  ss << "Renderer: " << gl()->glGetString(GL_RENDERER) << endl;
  ss << "Vendor: " << gl()->glGetString(GL_VENDOR) << endl;
  ss << "GLSH version: " << gl()->glGetString(GL_SHADING_LANGUAGE_VERSION)
     << endl;

  const GLubyte *str = gl()->glGetString(GL_EXTENSIONS);
  if (str)
    ss << "Extensions: " << str << endl;
  else
    ss << "Extensions:" << endl;

  logToFile(ss.str());

  if (isProgramOptionSet("glInfo")) {
    cout << ss.str();
    MyApplication::mainExit();
  }

  createContext();

  if (!glSharing) {
    cl_int err;
    commandQueue = clCreateCommandQueue(globalContext->getCLContext(),
                                        globalContext->getCLDevice(), 0, &err);
    checkClErrorCode(err, "clCreateCommandQueue()");
  }

  checkGLMessages();
}

void Canvas::resizeGL(int /*w*/, int /*h*/) {
  // checkGLMessages();
}

void Canvas::paintGL() {
  using namespace chrono;

  if (paintingDisabled)
    return;

#ifndef NDEBUG
  logToFile("Painting started.");
#endif

  gl()->glClear(GL_COLOR_BUFFER_BIT);

  if (ready()) {
    decltype(high_resolution_clock::now()) start;
    if (printTiming) {
      start = high_resolution_clock::now();
    }

    // Calculate the transformMatrix.
    const double ratio = virtualRatio();
    const float position = leftEdgePosition();

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(position, 0, width() * ratio, height()));
    // TODO: Shift the view so that the left edge is sways at (or near) 0.
    // Then there can be no problems with limited float range.

    // Set uniform variables.
    const vector<float> units = {1000 * 1000, 1000, 1, 0.001f, 0.001f * 0.001f};
    sampleScale = OpenDataFile::infoTable.getSampleScale() /
                  units[OpenDataFile::infoTable.getSampleUnits()];
    sampleScale /= physicalDpiY() / 2.54;

    gl()->glUseProgram(signalProgram->getGLProgram());
    setUniformTransformMatrix(signalProgram.get(), matrix.data());

    gl()->glUseProgram(eventProgram->getGLProgram());
    setUniformTransformMatrix(eventProgram.get(), matrix.data());
    setUniformEventWidth(eventProgram.get(),
                         0.45 * height() / signalProcessor->getTrackCount());

    gl()->glUseProgram(rectangleLineProgram->getGLProgram());
    setUniformTransformMatrix(rectangleLineProgram.get(), matrix.data());

    // Create the data block range needed.
    const int firstSample = static_cast<int>(floor(position));
    const int lastSample = static_cast<int>(ceil(position + width() * ratio));

    const auto fromTo =
        sampleRangeToBlockRange(firstSample, lastSample, nSamples);

    auto tr = SignalProcessor::blockIndexToSampleRange(fromTo.first, nSamples);
    assert(true || tr.first <= firstSample);
    tr = SignalProcessor::blockIndexToSampleRange(fromTo.second, nSamples);
    assert(true || lastSample <= tr.second);
    (void)tr; // TODO: Find out why these checks fail when synchronizing views.

    set<int> indexSet;

    for (int i = fromTo.first; i <= fromTo.second; ++i)
      indexSet.insert(i);

    // Get events.
    // TODO: This can take a long time when there are a lot of events on the
    // screen.
    // Perhaps it doesn't need to be done every time?
    vector<tuple<int, int, int>> allChannelEvents;
    vector<tuple<int, int, int, int>> singleChannelEvents;
    getEventsForRendering(file, firstSample, lastSample, &allChannelEvents,
                          &singleChannelEvents);

    // Draw.
    drawTimeLines();
    drawAllChannelEvents(allChannelEvents);

    int index;
    GPUCacheItem *cacheItem;

    while ((cacheItem = cache->getAny(indexSet, &index))) {
      drawBlock(index, cacheItem, singleChannelEvents);

      indexSet.erase(index);
    }

    gl()->glFlush();
    auto it = indexSet.begin();

    for (unsigned int i = 0; i < indexSet.size(); i += parallelQueues) {
      vector<int> indexVector;
      vector<cl_mem> bufferVector;
      vector<GPUCacheItem *> items;

      for (int j = 0; j < parallelQueues && j + i < indexSet.size(); ++j) {
        int index = *(it++);
        cacheItem = cache->setOldest(index);
        logToFile("Loading block " << index << " to GPU cache.");

        indexVector.push_back(index);
        bufferVector.push_back(glSharing ? cacheItem->sharedBuffer
                                         : processorOutputBuffers[j]);
        items.push_back(cacheItem);
      }

      signalProcessor->process(indexVector, bufferVector);

      if (!glSharing) {
        // Pull the data from CL buffer and copy it to the GL buffer.
        cl_int err;
        size_t size = signalProcessor->montageLength() *
                      signalProcessor->getTrackCount() * sizeof(float);
        if (duplicateSignal)
          size *= 2;

        for (unsigned int j = 0; j < indexVector.size(); ++j) {
          err = clEnqueueReadBuffer(
              commandQueue, processorOutputBuffers[j], CL_TRUE, 0, size,
              processorSyncBuffer.data(), 0, nullptr, nullptr);
          checkClErrorCode(err, "clEnqueueReadBuffer");

          gl()->glBindBuffer(GL_ARRAY_BUFFER, items[j]->signalBuffer);
          gl()->glBufferData(GL_ARRAY_BUFFER, size, processorSyncBuffer.data(),
                             GL_STATIC_DRAW);

          drawBlock(indexVector[j], items[j], singleChannelEvents);
        }
      } else {
        for (unsigned int j = 0; j < indexVector.size(); ++j)
          drawBlock(indexVector[j], items[j], singleChannelEvents);
      }

      gl()->glFlush();
    }

    drawPositionIndicator();
    drawCross();

    // gl3()->glBindVertexArray(0);

    if (printTiming) {
      const nanoseconds time = high_resolution_clock::now() - start;
      cerr << "Frame " << lastSample - firstSample << " samples long took "
           << static_cast<double>(time.count()) / 1000 / 1000 / 1000
           << " s to redraw\n";
    }
  }

  gl()->glFinish();

  checkGLMessages();

#ifndef NDEBUG
  logToFile("Painting finished.");
#endif
}

void Canvas::wheelEvent(QWheelEvent *event) {
  bool isDown;

  if (event->angleDelta().isNull() == false) {
    isDown = event->angleDelta().x() + event->angleDelta().y() < 0;
  } else if (event->pixelDelta().isNull() == false) {
    isDown = event->pixelDelta().x() + event->pixelDelta().y() < 0;
  } else {
    event->ignore();
    return;
  }

  if (event->modifiers() & Qt::ControlModifier) {
    trackZoom(isDown);

    update();
    event->accept();
    return;
  } else if (event->modifiers() & Qt::AltModifier) {
    horizontalZoom(isDown);

    updateCursor();
    // TODO: Is this needed.
    emit OpenDataFile::infoTable.positionChanged(
        OpenDataFile::infoTable.getPosition(),
        OpenDataFile::infoTable.getPositionIndicator());

    update();
    event->accept();
    return;
  } else if (event->modifiers() & Qt::ShiftModifier) {
    if (isDown)
      emit shiftZoomDown();
    else
      emit shiftZoomUp();

    update();
    event->accept();
    return;
  }

  event->ignore();
}

void Canvas::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Control) {
    if (isDrawingEvent == false) {
      isSelectingTrack = true;
      update();
    }
  } else if (event->key() == Qt::Key_C) {
    isDrawingCross =
        !isDrawingCross; // TODO: Perhaps promote this to an action?
    update();
  } else if (ready() && event->key() == Qt::Key_T) {
    updatePositionIndicator();
  } else {
    event->ignore();
  }
}

void Canvas::keyReleaseEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Control) {
    isSelectingTrack = false;
    update();
  }
}

void Canvas::mouseMoveEvent(QMouseEvent * /*event*/) {
  if (ready()) {
    updateCursor();

    if (isSelectingTrack || isDrawingEvent || isDrawingCross)
      update();
  }
}

void Canvas::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (event->modifiers() == Qt::ShiftModifier ||
        event->modifiers() == Qt::ControlModifier || isShiftChecked ||
        isCtrlChecked) {
      if (ready() && 0 <= cursorTrack &&
          cursorTrack < signalProcessor->getTrackCount()) {
        isDrawingEvent = true;

        eventStart = eventEnd =
            leftEdgePosition() + event->pos().x() * virtualRatio();

        if (event->modifiers() == Qt::ShiftModifier || isShiftChecked)
          eventTrack = -1;
        else if (event->modifiers() == Qt::ControlModifier || isCtrlChecked)
          eventTrack = cursorTrack;

        isSelectingTrack = false;

        update();
      }
    }
  }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (isDrawingEvent) {
      isDrawingEvent = false;
      addEvent(eventTrack);
      update();
    }
  }
}

void Canvas::focusOutEvent(QFocusEvent * /*event*/) {
  if (isSelectingTrack) {
    isSelectingTrack = false;
    update();
  } else if (isDrawingEvent) {
    isDrawingEvent = false;
    update();
  } else if (isDrawingCross) {
    update();
  }
}

void Canvas::focusInEvent(QFocusEvent * /*event*/) {}

double Canvas::virtualRatio() {
  return static_cast<double>(file->file->getSamplesRecorded()) /
         OpenDataFile::infoTable.getVirtualWidth();
}

float Canvas::leftEdgePosition() {
  return OpenDataFile::infoTable.getPosition() -
         OpenDataFile::infoTable.getPositionIndicator() * width() *
             virtualRatio();
}

void Canvas::updateProcessor() {
  cl_int err;

  cache.reset(nullptr);

  if (!glSharing) {
    for (cl_mem e : processorOutputBuffers) {
      err = clReleaseMemObject(e);
      checkClErrorCode(err, "clReleaseMemObject()");
    }
    processorOutputBuffers.clear();
  }
  assert(processorOutputBuffers.empty());

  if (!ready())
    return;

  nSamples =
      signalProcessor->montageLength() - (extraSamplesFront + extraSamplesBack);

  size_t size = signalProcessor->montageLength() *
                signalProcessor->getTrackCount() * sizeof(float);
  if (duplicateSignal)
    size *= 2;

  int64_t gpuMemorySize = programOption<int>("gpuMemorySize");
  gpuMemorySize *= 1000 * 1000; // Convert from MB.

  if (gpuMemorySize <= 0) {
    cl_ulong gpuSize;
    cl_int err =
        clGetDeviceInfo(globalContext->getCLDevice(), CL_DEVICE_GLOBAL_MEM_SIZE,
                        sizeof(cl_ulong), &gpuSize, nullptr);
    checkClErrorCode(err, "clGetDeviceInfo()");

    // The desktop environment usually needs some memory to work with, so we
    // leave it 25%.
    gpuMemorySize = gpuSize / 4 * 3;
  }

  int cacheCapacity = static_cast<int>(gpuMemorySize / size);

  // SignalProcessor uses 2 temporary buffers plus 1 FilterProcessor per queue.
  // Additionally when sharing is turned off another set of output buffers is
  // needed.
  // So lessen the capacity to make sure there is enough space for these
  // objects.
  cacheCapacity -= parallelQueues * (glSharing ? 3 : 4);

  if (cacheCapacity <= 0)
    throwDetailed(runtime_error(
        "Not enough GPU memory to create cache with non zero capacity."));

  logToFile("Creating GPU cache with "
            << cacheCapacity << " capacity and blocks of size " << size << ".");

  cache = make_unique<LRUCache<int, GPUCacheItem>>(
      cacheCapacity,
      make_unique<GPUCacheAllocator>(size, duplicateSignal, glSharing,
                                     globalContext.get()));

  if (!glSharing) {
    processorSyncBuffer.resize(size / sizeof(float));

    for (int i = 0; i < parallelQueues; ++i) {
      cl_mem_flags flags = CL_MEM_READ_WRITE;

      processorOutputBuffers.push_back(clCreateBuffer(
          globalContext->getCLContext(), flags, size, nullptr, &err));
      checkClErrorCode(err, "clCreateBuffer()");
    }
  }

  update();
}

void Canvas::drawBlock(
    int index, GPUCacheItem *cacheItem,
    const vector<tuple<int, int, int, int>> &singleChannelEvents) {
  assert(cacheItem);

  signalArray = cacheItem->signalArray;
  signalBuffer = cacheItem->signalBuffer;
  eventArray = cacheItem->eventArray;

  drawSingleChannelEvents(index, singleChannelEvents);
  drawSignal(index);
}

void Canvas::drawAllChannelEvents(
    const vector<tuple<int, int, int>> &eventVector) {
  gl()->glUseProgram(rectangleLineProgram->getGLProgram());
  bindArray(rectangleLineArray, rectangleLineBuffer, 2, 0);
  gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

  int event = 0, type = -1;
  while (event < static_cast<int>(eventVector.size())) {
    if (type == get<0>(eventVector[event])) {
      int from = get<1>(eventVector[event]);
      int to = from + get<2>(eventVector[event]) - 1;

      drawAllChannelEvent(from, to);

      ++event;
    } else {
      type = get<0>(eventVector[event]);
      QColor color;
      double opacity;
      getEventTypeColorOpacity(file, type, &color, &opacity);
      setUniformColor(rectangleLineProgram->getGLProgram(), color, opacity);
    }
  }

  if (isDrawingEvent) {
    if (eventTrack == -1) {
      QColor color(Qt::blue);
      double opacity = 0.5;

      int type = OpenDataFile::infoTable.getSelectedType();
      if (type != -1)
        getEventTypeColorOpacity(file, type, &color, &opacity);

      setUniformColor(rectangleLineProgram->getGLProgram(), color, opacity);

      int start = eventStart, end = eventEnd;

      if (end < start)
        swap(start, end);

      drawAllChannelEvent(start, end);
    }
  }
}

void Canvas::drawAllChannelEvent(int from, int to) {
  float data[8] = {static_cast<float>(from), 0,
                   static_cast<float>(to),   0,
                   static_cast<float>(from), static_cast<float>(height()),
                   static_cast<float>(to),   static_cast<float>(height())};

  gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  gl()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::drawTimeLines() {
  const double interval = OpenDataFile::infoTable.getTimeLineInterval() *
                          file->file->getSamplingFrequency();

  if (interval > 0) {
    gl()->glUseProgram(rectangleLineProgram->getGLProgram());
    bindArray(rectangleLineArray, rectangleLineBuffer, 2, 0);
    gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

    setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::green), 1);

    const double position = leftEdgePosition();
    const double end = position + width() * virtualRatio();

    double nextPosition = ceil(position / interval) * interval;

    while (nextPosition <= end) {
      drawTimeLine(nextPosition);

      nextPosition += interval;
    }
  }
}

void Canvas::drawPositionIndicator() {
  gl()->glUseProgram(rectangleLineProgram->getGLProgram());
  bindArray(rectangleLineArray, rectangleLineBuffer, 2, 0);
  gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

  setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::blue), 1);

  const double position = OpenDataFile::infoTable.getPosition();
  drawTimeLine(position);
}

void Canvas::drawCross() {
  const QPoint pos = mapFromGlobal(QCursor::pos());

  if (isDrawingCross == false || hasFocus() == false || pos.x() < 0 ||
      pos.x() >= width() || pos.y() < 0 || pos.y() >= height()) {
    return;
  }

  gl()->glUseProgram(rectangleLineProgram->getGLProgram());
  bindArray(rectangleLineArray, rectangleLineBuffer, 2, 0);
  gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

  setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::black), 1);

  const float position = leftEdgePosition() + pos.x() * virtualRatio();
  const auto samplesRecorded = file->file->getSamplesRecorded();

  const float data[8] = {position,
                         0,
                         position,
                         static_cast<float>(height()),
                         0,
                         static_cast<float>(pos.y()),
                         static_cast<float>(samplesRecorded),
                         static_cast<float>(pos.y())};

  gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  gl()->glDrawArrays(GL_LINE_STRIP, 0, 2);
  gl()->glDrawArrays(GL_LINE_STRIP, 2, 2);
}

void Canvas::drawTimeLine(const double position) {
  float data[4] = {static_cast<float>(position), 0,
                   static_cast<float>(position), static_cast<float>(height())};

  gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  gl()->glDrawArrays(GL_LINE_STRIP, 0, 2);
}

void Canvas::drawSingleChannelEvents(
    int index, const vector<tuple<int, int, int, int>> &eventVector) {
  gl()->glUseProgram(eventProgram->getGLProgram());

  bindArray(eventArray, signalBuffer, 1, 0);

  const AbstractTrackTable *trackTable = getTrackTable(file);
  int event = 0, type = -1, track = -1, hidden = 0;

  while (event < static_cast<int>(eventVector.size())) {
    if (type == get<0>(eventVector[event])) {
      if (track == get<1>(eventVector[event])) {
        int from = get<2>(eventVector[event]);
        int to = from + get<3>(eventVector[event]) - 1;

        drawSingleChannelEvent(index, track - hidden, from, to);

        ++event;
      } else {
        track = get<1>(eventVector[event]);
        assert(trackTable->row(track).hidden == false);

        hidden = 0;
        for (int i = 0; i < track; ++i) {
          if (trackTable->row(i).hidden)
            ++hidden;
        }

        setUniformTrack(eventProgram->getGLProgram(), track, hidden, index);
      }
    } else {
      type = get<0>(eventVector[event]);
      QColor color;
      double opacity;
      getEventTypeColorOpacity(file, type, &color, &opacity);
      setUniformColor(eventProgram->getGLProgram(), color, opacity);
    }
  }

  if (isDrawingEvent) {
    int track = eventTrack;

    if (0 <= track && track < signalProcessor->getTrackCount()) {
      int hidden = countHiddenTracks(track);

      setUniformTrack(eventProgram->getGLProgram(), track + hidden, hidden,
                      index);

      QColor color(Qt::blue);
      double opacity = 0.5;
      int type = OpenDataFile::infoTable.getSelectedType();
      if (type != -1)
        getEventTypeColorOpacity(file, type, &color, &opacity);
      setUniformColor(eventProgram->getGLProgram(), color, opacity);

      int start = eventStart, end = eventEnd;

      if (end < start)
        swap(start, end);

      drawSingleChannelEvent(index, track, start, end);
    }
  }
}

void Canvas::drawSingleChannelEvent(int index, int track, int from, int to) {
  auto fromTo = SignalProcessor::blockIndexToSampleRange(index, nSamples);
  int firstSample = fromTo.first;
  int lastSample = fromTo.second;

  if (from <= lastSample && firstSample <= to) {
    from = max(firstSample, from);
    to = min(lastSample, to);

    gl()->glDrawArrays(
        GL_TRIANGLE_STRIP,
        2 * (track * signalProcessor->montageLength() + from - firstSample),
        2 * (to - from + 1));
  }
}

void Canvas::drawSignal(int index) {
  gl()->glUseProgram(signalProgram->getGLProgram());

  bindArray(signalArray, signalBuffer, 1,
            duplicateSignal ? 2 * sizeof(float) : 0);

  for (int track = 0; track < signalProcessor->getTrackCount(); ++track) {
    int hidden = countHiddenTracks(track);

    setUniformTrack(signalProgram->getGLProgram(), track + hidden, hidden,
                    index);

    QColor color = DataModel::array2color<QColor>(
        getTrackTable(file)->row(track + hidden).color);
    if (isSelectingTrack && track == cursorTrack)
      color = modifySelectionColor(color);

    setUniformColor(signalProgram->getGLProgram(), color, 1);

    gl()->glDrawArrays(GL_LINE_STRIP, track * signalProcessor->montageLength(),
                       nSamples);
  }
}

void Canvas::setUniformTrack(GLuint program, int track, int hidden, int index) {
  GLuint location = gl()->glGetUniformLocation(program, "y0");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  float y0 =
      (track - hidden + 0.5f) * height() / signalProcessor->getTrackCount();
  gl()->glUniform1f(location, y0);

  location = gl()->glGetUniformLocation(program, "yScale");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  float yScale = getTrackTable(file)->row(track).amplitude;
  gl()->glUniform1f(location, -1 * yScale / sampleScale);

  location = gl()->glGetUniformLocation(program, "bufferOffset");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  gl()->glUniform1i(
      location,
      SignalProcessor::blockIndexToSampleRange(index, nSamples).first -
          (track - hidden) * signalProcessor->montageLength());
}

void Canvas::setUniformColor(GLuint program, const QColor &color,
                             double opacity) {
  double r, g, b, a;
  color.getRgbF(&r, &g, &b, &a);
  a = opacity;

  GLuint location = gl()->glGetUniformLocation(program, "color");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  gl()->glUniform4f(location, r, g, b, a);
}

void Canvas::checkGLMessages() {
  auto logPtr = OPENGL_INTERFACE->log();

  if (logPtr) {
    for (const auto &m : logPtr->loggedMessages()) {
      string message = m.message().toStdString();

      if (message != lastGLMessage) {
        logLastGLMessage();
        lastGLMessageCount = 0;

        logToFile("OpenGL message: " << message);
      }

      lastGLMessage = message;
      ++lastGLMessageCount;
    }
  }
}

void Canvas::addEvent(int channel) {
  file->undoFactory->beginMacro("add event");

  const AbstractEventTable *eventTable = getEventTable(file);

  int index = eventTable->rowCount();
  file->undoFactory->insertEvent(OpenDataFile::infoTable.getSelectedMontage(),
                                 index);

  Event e = eventTable->row(index);

  e.type = OpenDataFile::infoTable.getSelectedType();
  if (eventEnd < eventStart)
    swap(eventStart, eventEnd);
  e.position = eventStart;
  e.duration = eventEnd - eventStart + 1;
  e.channel = channel + countHiddenTracks(channel);

  file->undoFactory->changeEvent(OpenDataFile::infoTable.getSelectedMontage(),
                                 index, e);

  file->undoFactory->endMacro();
}

int Canvas::countHiddenTracks(int track) {
  int hidden = 0;
  int i = 0;
  assert(track < getTrackTable(file)->rowCount());

  while (i - hidden <= track) {
    if (getTrackTable(file)->row(i++).hidden)
      ++hidden;
  }

  return hidden;
}

void Canvas::createContext() {
  // TODO: Make sure this function is executed only once.
  vector<cl_context_properties> properties;

  if (glSharing) {
#ifndef __APPLE__
    properties.push_back(CL_GL_CONTEXT_KHR);

#if defined WIN_BUILD
    properties.push_back(
        reinterpret_cast<cl_context_properties>(wglGetCurrentContext()));

    properties.push_back(CL_WGL_HDC_KHR);
    properties.push_back(
        reinterpret_cast<cl_context_properties>(wglGetCurrentDC()));
#elif defined UNIX_BUILD
    properties.push_back(
        reinterpret_cast<cl_context_properties>(glXGetCurrentContext()));

    properties.push_back(CL_GLX_DISPLAY_KHR);
    properties.push_back(
        reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()));
#endif
#endif
  }

  globalContext = make_unique<AlenkaSignal::OpenCLContext>(
      programOption<int>("clPlatform"), programOption<int>("clDevice"),
      properties);

  if (programOption<bool>("kernelCachePersist"))
    OpenDataFile::kernelCache->loadFromFile(globalContext.get());
}

void Canvas::setUniformTransformMatrix(OpenGLProgram *program, float *data) {
  GLuint location =
      gl()->glGetUniformLocation(program->getGLProgram(), "transformMatrix");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  gl()->glUniformMatrix4fv(location, 1, GL_FALSE, data);
}

void Canvas::setUniformEventWidth(OpenGLProgram *program, float value) {
  GLuint location =
      gl()->glGetUniformLocation(program->getGLProgram(), "eventWidth");
  checkNotErrorCode(location, static_cast<GLuint>(-1),
                    "glGetUniformLocation() failed.");
  gl()->glUniform1f(location, value);
}

void Canvas::logLastGLMessage() {
  if (lastGLMessageCount > 1) {
    logToFile("OpenGL message (" << lastGLMessageCount - 1
                                 << "x): " << lastGLMessage);
  }
}

void Canvas::updatePositionIndicator() {
  const QPoint pos = mapFromGlobal(QCursor::pos());
  const double indicator = static_cast<double>(pos.x()) / width();

  const double oldIndicator = OpenDataFile::infoTable.getPositionIndicator();
  const double oldPosition = OpenDataFile::infoTable.getPosition();

  // We must correct the position so that the window doesn't move when the
  // indicator changes.
  const double newPosition =
      oldPosition + (indicator - oldIndicator) * width() * virtualRatio();

  // For extremely small canvases (e.g. with width 0) it doesn't make sense to
  // set the indicator.
  if (0 <= indicator && indicator <= 1) {
    OpenDataFile::infoTable.setPosition(static_cast<int>(round(newPosition)),
                                        indicator);
    update();
  }
}

void Canvas::updateFilter() {
  assert(signalProcessor);

  makeCurrent();
  signalProcessor->updateFilter();
  updateProcessor();
  doneCurrent();
}

void Canvas::selectMontage() {
  for (auto e : montageConnections)
    disconnect(e);
  montageConnections.clear();

  if (file) {
    assert(0 < file->dataModel->montageTable()->rowCount());

    auto trackTable = getTrackTable(file);
    auto vitness = VitnessTrackTable::vitness(trackTable);

    auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this,
                     SLOT(updateMontage(int, int)));
    montageConnections.push_back(c);

    c = connect(vitness, SIGNAL(rowsInserted(int, int)), this,
                SLOT(updateMontage()));
    montageConnections.push_back(c);

    c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
                SLOT(updateMontage()));
    montageConnections.push_back(c);

    // Also connect the default montage, so that the view gets updated when
    // using undo/redo and a non-default montage, that depends on the default
    // one via labels or coordinates, is selected.
    auto defaultTrackTable = file->dataModel->montageTable()->trackTable(0);
    if (trackTable != defaultTrackTable) {
      auto defaultVitness = VitnessTrackTable::vitness(defaultTrackTable);
      c = connect(defaultVitness, SIGNAL(valueChanged(int, int)), this,
                  SLOT(updateMontage()));
      montageConnections.push_back(c);
    }

    // Also connect the global OpenCL header so that the montage gets recompiled
    // every time it changes.
    c = connect(&OpenDataFile::infoTable,
                SIGNAL(globalMontageHeaderChanged(QString)), this,
                SLOT(updateMontage()));
    montageConnections.push_back(c);
  }

  updateMontage();
}

void Canvas::updateMontage(int /*row*/, int col) {
  Track::Index column = static_cast<Track::Index>(col);
  if (column == Track::Index::code || column == Track::Index::hidden)
    updateMontage();
}

void Canvas::updateMontage() {
  if (signalProcessor) {
    makeCurrent();
    signalProcessor->setUpdateMontageFlag();
    updateProcessor();
    doneCurrent();
  }
}

bool Canvas::ready() { return signalProcessor && signalProcessor->ready(); }
