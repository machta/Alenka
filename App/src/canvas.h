#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"

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

	template <typename T>
	std::set<T> createSetFromRange(T from, T to)
	{
		std::set<unsigned int> s;
		for (int i = from; i <= to; ++i)
		{
			s.insert(i);
		}

		return s;
	}
};

#endif // CANVAS_H
