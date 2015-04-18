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

class Canvas : public QOpenGLWidget, public OpenGLInterface
{
	Q_OBJECT

public:
	explicit Canvas(QWidget* parent = nullptr);
	~Canvas();

	void changeFile(DataFile* file);
	int getCursorPositionSample() const
	{
		return cursorSample;
	}
	int getCursorPositionTrack() const
	{
		return cursorTrack;
	}

	static QColor modifySelectionColor(const QColor& color);

signals:
	void cursorPositionSampleChanged(int);
	void cursorPositionTrackChanged(int);

public slots:
	void updateCursor();
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
	void prepareBlocks(int from, int to)
	{
		for (; from <= to; ++from)
		{
			signalProcessor->prepareBlock(from);
		}
	}
	void prepare(int size, int minIndex, int maxIndex, int lowIndex, int highIndex, int capacity)
	{
		using namespace std;

		if (capacity <= 0)
		{
			return;
		}

		prepare(size, minIndex, maxIndex, lowIndex - size, highIndex + size, capacity - 2*size);

		capacity = min(2*size, capacity);

		prepareBlocks(max(lowIndex, minIndex), lowIndex + capacity/2);
		prepareBlocks(highIndex, min(highIndex, maxIndex) + capacity/2);
	}
	void horizontalZoom(double factor);
	void verticalZoom(double factor);
	void trackZoom(double factor);
	void addEvent(int channel);
	int countHiddenTracks(int track);

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
	void updateMontage(const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		if (bottomRight.row() - topLeft.row() >= 0)
		{
			int column = static_cast<int>(TrackTable::Column::code);
			if (topLeft.column() <= column && column <= bottomRight.column())
			{
				updateMontage();
				return;
			}

			column = static_cast<int>(TrackTable::Column::hidden);
			if (topLeft.column() <= column && column <= bottomRight.column())
			{
				updateMontage();
				return;
			}
		}
	}
	void updateMontage()
	{
		assert(signalProcessor != nullptr);

		makeCurrent();
		signalProcessor->updateMontage();
		doneCurrent();
	}
};

#endif // CANVAS_H
