#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"
#include "DataFile/datafile.h"

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

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

private:
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	TrackTable* montageTable;
	EventTable* eventTable;
	EventTypeTable* eventTypeTable;
	SignalProcessor* signalProcessor = nullptr;
	OpenGLProgram* signalProgram = nullptr;
	OpenGLProgram* eventProgram = nullptr;
	OpenGLProgram* rectangleProgram = nullptr;
	double samplesRecorded = 1;
	GLuint rectangleArray;
	GLuint rectangleBuffer;
	int eventMode = PROGRAM_OPTIONS["eventRenderMode"].as<int>();

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
	void drawBlock(const SignalBlock& block, const std::vector<std::tuple<int, int, int, int>>& singleChannelEvents);
	void setUniformChannel(GLuint program, int channel, const SignalBlock& block);
	void setUniformColor(GLuint program, const QColor& color, float opacity);
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

private slots:
	void changeFilter()
	{
		signalProcessor->changeFilter();
	}
};

#endif // CANVAS_H
