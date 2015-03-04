#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"

#include <algorithm>

class Canvas : public QOpenGLWidget, public OpenGLInterface
{
	Q_OBJECT

public:
	explicit Canvas(QWidget* parent = 0);
	~Canvas();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

private:
	SignalProcessor* signalProcessor = nullptr;
	OpenGLProgram* program = nullptr;
	DataFile* dataFile = nullptr;

	double samplePixelRatio();
	void paintBlock(const SignalBlock& block);
	void paintChannel(unsigned int channel, const SignalBlock& block);
	void checkGLMessages()
	{
		for (const auto& m : log()->loggedMessages())
		{
			std::cerr << "OpenGL message: " << m.message().toStdString() << std::endl;
		}
	}
	void prepareBlocks(int from, int to)
	{
		for (; from <= to; ++from)
		{
			signalProcessor->prepareBlocks(from);
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
