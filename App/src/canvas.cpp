#include "canvas.h"

#include "options.h"
#include "DataFile/gdf2.h"
#include "signalviewer.h"

#include <string>
#include <cmath>
#include <set>

using namespace std;

Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent)
{

}

Canvas::~Canvas()
{
	delete signalProcessor;
	delete program;
	delete dataFile;

	fun()->glDeleteVertexArrays(1, &vertexArray);
}

void Canvas::initializeGL()
{
	dataFile = new GDF2(PROGRAM_OPTIONS->get("file").as<string>().c_str());

	signalProcessor = new SignalProcessor(dataFile);

	program = new OpenGLProgram(PROGRAM_OPTIONS->get("vert").as<string>().c_str(),
	                            PROGRAM_OPTIONS->get("frag").as<string>().c_str());

	fun()->glGenVertexArrays(1, &vertexArray);
	fun()->glBindVertexArray(vertexArray);

	fun()->glClearColor(1, 1, 1, 1);

	checkGLMessages();
}

void Canvas::resizeGL(int w, int h)
{
	const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());

	double ratio = samplePixelRatio();
	QRectF rect(parent->getPosition()*ratio, 0, w*ratio, h);

	QMatrix4x4 matrix;
	matrix.ortho(rect);

	GLuint location = fun()->glGetUniformBlockIndex(program->getGLProgram(), "transformMatrix");
	fun()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

	checkGLMessages();
}

void Canvas::paintGL()
{
	fun()->glClear(GL_COLOR_BUFFER_BIT);

	fun()->glUseProgram(program->getGLProgram());
	fun()->glBindVertexArray(vertexArray);

	const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());

	double ratio = samplePixelRatio();

	int firstIndex = static_cast<int>(floor(parent->getPosition()*ratio)),
	    lastIndex = static_cast<int>(ceil((parent->getPosition()+width())*ratio));

	firstIndex /= signalProcessor->getBlockSize();
	lastIndex /= signalProcessor->getBlockSize();

	set<unsigned int> indexSet;
	for (int i = firstIndex; i <= lastIndex; ++i)
	{
		indexSet.insert(i);
	}

	while (indexSet.empty() == false)
	{
		SignalBlock block = signalProcessor->getAnyBlock(indexSet);
		paintBlock(block);
		indexSet.erase(block.getIndex());
	}

	fun()->glFlush();
	fun()->glFinish();

	checkGLMessages();
}

double Canvas::samplePixelRatio()
{
	const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());

	double tmp = dataFile->getSamplesRecorded();
	tmp /= parent->getVirtualWidth();

	return tmp;
}

void Canvas::paintBlock(const SignalBlock& block)
{
	fun()->glBindBuffer(GL_ARRAY_BUFFER, block.geGLBuffer());
	fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	//fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	fun()->glEnableVertexAttribArray(0);

	GLuint location = fun()->glGetUniformBlockIndex(program->getGLProgram(), "firstSample");
	fun()->glUniform1i(location, block.getFirstSample());

	for (unsigned int i = 0; i < block.getchannelCount(); ++i)
	{
		paintChannel(i, block);
	}
}

void Canvas::paintChannel(unsigned int channel, const SignalBlock& block)
{
	GLuint location = fun()->glGetUniformBlockIndex(program->getGLProgram(), "y0");
	fun()->glUniform1f(location, (channel + 0.5f)*height()/block.getchannelCount());

	location = fun()->glGetUniformBlockIndex(program->getGLProgram(), "yScale");
	fun()->glUniform1f(location, 0.1f);

	GLint size = block.getLastSample() - block.getFirstSample() + 1;
	fun()->glDrawArrays(GL_LINE_STRIP, channel*size, size);
}
