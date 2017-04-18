#ifndef CANVAS_H
#define CANVAS_H

#include "openglinterface.h"
#include "SignalProcessor/lrucache.h"

#include <CL/cl_gl.h>

#include <QOpenGLWidget>

#include <cassert>
#include <vector>
#include <tuple>

namespace AlenkaSignal
{
class OpenCLContext;
}
class OpenDataFile;
class SignalProcessor;
class SignalBlock;
class OpenGLProgram;

struct GPUCacheItem
{
	GLuint signalBuffer, signalArray, eventArray;
	cl_mem sharedBuffer;
};

/**
 * @brief This class implements GUI control for rendering signal data and events.
 *
 * Every time this control gets updated the whole surface gets repainted.
 *
 * This class also handles user keyboard and mouse input. Scrolling related
 * events are skipped and handled by the parent.
 *
 *
 */
class Canvas : public QOpenGLWidget, public OpenGLInterface
{
	Q_OBJECT

	OpenDataFile* file = nullptr;
	SignalProcessor* signalProcessor = nullptr;
	OpenGLProgram* signalProgram = nullptr;
	OpenGLProgram* eventProgram = nullptr;
	OpenGLProgram* rectangleLineProgram = nullptr;
	double samplesRecorded = 1;
	double samplingFrequency = 1;
	GLuint rectangleLineArray;
	GLuint rectangleLineBuffer;
	GLuint signalArray;
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
	LRUCache<int, GPUCacheItem>* cache = nullptr;
	AlenkaSignal::OpenCLContext* context = nullptr;
	int nBlock, nMontage, nSamples, M;
	int extraSamplesFront, extraSamplesBack;
	bool duplicateSignal, glSharing;
	unsigned int parallelQueues;
	cl_command_queue commandQueue = nullptr;
	float* processorSyncBuffer = nullptr;
	std::vector<cl_mem> processorOutputBuffers;

public:
	explicit Canvas(QWidget* parent = nullptr);
	~Canvas();

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(OpenDataFile* file);

	int getCursorPositionSample() const
	{
		return cursorSample;
	}
	int getCursorPositionTrack() const
	{
		return cursorTrack;
	}

	/**
	 * @brief This method defines logic of changing color of objects during selection.
	 *
	 * This method is used at different places in code.
	 * It is defined here so that this behavior is uniform.
	 *
	 * @todo The ad hoc method used to achieve this is not ideal and could be improved.
	 * All three components or are modified the same way. For colors that have two
	 * of the three components equal to zero or for black the resulting color differs little
	 * from the original color and the selected object is not very distinctive.
	 */
	static QColor modifySelectionColor(const QColor& color);

	void horizontalZoom(bool reverse);
	void verticalZoom(bool reverse);
	void trackZoom(bool reverse);

signals:
	void cursorPositionSampleChanged(int);
	void cursorPositionTrackChanged(int);

public slots:
	void updateCursor();

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void focusOutEvent(QFocusEvent* event) override;
	virtual void focusInEvent(QFocusEvent* event) override;

private:
	void updateProcessor();
	void drawBlock(int index, GPUCacheItem* cacheItem, const std::vector<std::tuple<int, int, int, int>>& singleChannelEvents);
	void drawAllChannelEvents(const std::vector<std::tuple<int, int, int>>& events);
	void drawAllChannelEvent(int from, int to);
	void drawTimeLines();
	void drawPositionIndicator();
	void drawCross();
	void drawTimeLine(double at);
	void drawSingleChannelEvents(int index, const std::vector<std::tuple<int, int, int, int>>& events);
	void drawSingleChannelEvent(int index, int track, int from, int to);
	void drawSignal(int index);
	void setUniformTrack(GLuint program, int track, int hidden, int index);
	void setUniformColor(GLuint program, const QColor& color, double opacity);
	void checkGLMessages();
	void addEvent(int channel);
	int countHiddenTracks(int track);
	void setCursorPositionSample(int sample)
	{
		if (sample != cursorSample)
		{
			cursorSample = sample;
			emit cursorPositionSampleChanged(sample);
		}
	}
	void setCursorPositionTrack(int track)
	{
		if (track != cursorTrack)
		{
			cursorTrack = track;
			emit cursorPositionTrackChanged(track);
		}
	}
	void createContext();

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
