#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"
#include "DataFile/datafile.h"
#include "DataFile/montagetable.h"
#include "DataFile/eventtypetable.h"
#include "DataFile/tracktable.h"
#include "DataFile/eventtable.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <tuple>

/**
 * @brief This class implemets GUI control for rendering signal data and events.
 *
 * Every time this control gets updated the whole surface gets repainted.
 *
 * This class also handles user keyboard and mouse input.
 */
class Canvas : public QOpenGLWidget, public OpenGLInterface
{
	Q_OBJECT

public:
	explicit Canvas(QWidget* parent = nullptr);
	~Canvas();

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(DataFile* file);

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
	 * @bug{The ad hoc method used to achieve this is not ideal and could be improved.
	 * All three components or are modified the same way. For colors that have two
	 * of the three components equal to zero or for black the resulting color differs little
	 * from the original color and the selected object is not very distinctive.}
	 */
	static QColor modifySelectionColor(const QColor& color);

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
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	MontageTable* montageTable = nullptr;
	EventTypeTable* eventTypeTable = nullptr;
	SignalProcessor* signalProcessor = nullptr;
	OpenGLProgram* signalProgram = nullptr;
	OpenGLProgram* eventProgram = nullptr;
	OpenGLProgram* rectangleLineProgram = nullptr;
	double samplesRecorded = 1;
	double samplingFrequency = 1;
	GLuint rectangleLineArray;
	GLuint rectangleLineBuffer;
	int eventMode = PROGRAM_OPTIONS["eventRenderMode"].as<int>();
	bool isSelectingTrack = false;
	bool isDrawingEvent = false;
	bool isDrawingCross = false;
	int eventTrack;
	int eventStart;
	int eventEnd;
	int cursorSample = 0;
	int cursorTrack = 0;

	InfoTable* getInfoTable()
	{
		if (infoTable != nullptr)
		{
			return infoTable;
		}
		else
		{
			return &defaultInfoTable;
		}
	}
	EventTable* currentEventTable()
	{
		return montageTable->getEventTables()->at(getInfoTable()->getSelectedMontage());
	}
	TrackTable* currentTrackTable()
	{
		return montageTable->getTrackTables()->at(getInfoTable()->getSelectedMontage());
	}
	void drawAllChannelEvents(const std::vector<std::tuple<int, int, int>>& events);
	void drawAllChannelEvent(int from, int to);
	void drawTimeLines();
	void drawPositionIndicator();
	void drawCross();
	void drawTimeLine(double at);
	void drawSingleChannelEvents(const SignalBlock& block, const std::vector<std::tuple<int, int, int, int>>& events);
	void drawSingleChannelEvent(const SignalBlock& block, int track, int from, int to);
	void drawSignal(const SignalBlock& block);
	void setUniformTrack(GLuint program, int track, int hidden, const SignalBlock& block);
	void setUniformColor(GLuint program, const QColor& color, double opacity);
	void checkGLMessages()
	{
		for (const auto& m : log()->loggedMessages())
		{
			logToFile("OpenGL message: " << m.message().toStdString());
		}
	}
	void horizontalZoom(double factor);
	void verticalZoom(double factor);
	void trackZoom(double factor);
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

private slots:
	void updateFilter()
	{
		assert(signalProcessor != nullptr);

		makeCurrent();
		signalProcessor->updateFilter();
		doneCurrent();
	}
	void selectMontage()
	{
		connect(currentTrackTable(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(updateMontage(QModelIndex, QModelIndex)));
		connect(currentTrackTable(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(updateMontage()));
		connect(currentTrackTable(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(updateMontage()));

		updateMontage();
	}
	void updateMontage(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	void updateMontage()
	{
		assert(signalProcessor != nullptr);

		makeCurrent();
		signalProcessor->setUpdateMontageFlag();
		doneCurrent();
	}

	/**
	 * @brief Tests whether SignalProcessor is ready to return blocks.
	 *
	 * This method is used to skip some code that would break if no file is
	 * open and/or the current montage is empty.
	 */
	bool ready()
	{
		return signalProcessor != nullptr && signalProcessor->ready();
	}
};

#endif // CANVAS_H
