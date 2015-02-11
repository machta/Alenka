#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include "openglinterface.h"

#include "SignalProcessor/signalprocessor.h"
#include "openglprogram.h"

#include <QMatrix4x4>

#include <QList>

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
	OpenGLProgram* program;
	DataFile* dataFile;
	GLuint vertexArray;

	double samplePixelRatio();
	void paintBlock(const SignalBlock& block);
	void paintChannel(unsigned int channel, const SignalBlock& block);
	void checkGLMessages()
	{
		for (auto m : log()->loggedMessages())
		{
			std::cerr << "OpenGL message: " << m.message().toStdString() << endl;
		}
	}
};

#endif // CANVAS_H
