#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"
#include "DataFile/datafile.h"

#include <algorithm>
#include <cassert>

class Canvas : public QOpenGLWidget, public OpenGLInterface
{
	Q_OBJECT

public:
	explicit Canvas(QWidget* parent = 0);
	~Canvas();

	void changeFile(DataFile* file)
	{
		makeCurrent();

		assert(signalProcessor != nullptr);

		signalProcessor->changeFile(file);

		if (file != nullptr)
		{
			samplesRecorded = file->getSamplesRecorded();
		}

		doneCurrent();
	}

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

private:
	SignalProcessor* signalProcessor = nullptr;
	OpenGLProgram* program = nullptr;
	double samplesRecorded = 1;

	double samplePixelRatio();
	void paintBlock(const SignalBlock& block);
	void paintChannel(unsigned int channel, const SignalBlock& block);
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
};

#endif // CANVAS_H
