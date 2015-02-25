#include "canvas.h"

#include "options.h"
#include "DataFile/gdf2.h"
#include "signalviewer.h"

#include <QMatrix4x4>

#include <cstdio>
#include <string>
#include <cmath>
#include <set>

using namespace std;

#define fun() fun_shortcut()

Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent)
{
	int dummy = 5;
	(void)dummy;
}

Canvas::~Canvas()
{
	delete signalProcessor;
	delete program;
	delete dataFile;
}

void Canvas::initializeGL()
{
	dataFile = new GDF2(PROGRAM_OPTIONS["file"].as<string>().c_str());

	signalProcessor = new SignalProcessor(dataFile);

	FILE* file1 = fopen(PROGRAM_OPTIONS["vert"].as<string>().c_str(), "rb");
	checkNotErrorCode(file1, nullptr, "File '" << PROGRAM_OPTIONS["vert"].as<string>() << "' could not be opened.");

	FILE* file2 = fopen(PROGRAM_OPTIONS["frag"].as<string>().c_str(), "rb");
	checkNotErrorCode(file2, nullptr, "File '" << PROGRAM_OPTIONS["frag"].as<string>() << "' could not be opened.");

	program = new OpenGLProgram(file1, file2);

	fclose(file1);
	fclose(file2);

	fun()->glUseProgram(program->getGLProgram());

	fun()->glClearColor(1, 1, 1, 1);

	checkGLMessages();
}

void Canvas::resizeGL(int /*w*/, int /*h*/)
{
	int dummy = 5;
	(void)dummy;

	//const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());

	checkGLMessages();
}

void Canvas::paintGL()
{
	fun()->glClear(GL_COLOR_BUFFER_BIT);

	// Calculate the transformMatrix.
	const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());
	double ratio = samplePixelRatio();

	QRectF rect(parent->getPosition()*ratio, 0, width()*ratio, height());

	QMatrix4x4 matrix;
	matrix.ortho(rect);

	GLuint location = fun()->glGetUniformLocation(program->getGLProgram(), "transformMatrix");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	fun()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

	// Create the data block range needed.
	int firstIndex = static_cast<unsigned int>(floor(parent->getPosition()*ratio)),
		lastIndex = static_cast<unsigned int>(ceil((parent->getPosition()+width())*ratio));

	firstIndex /= signalProcessor->getBlockSize();
	lastIndex /= signalProcessor->getBlockSize();

	set<int> indexSet = createSetFromRange(firstIndex, lastIndex);

	// Notify the signal processor to prepare some blocks.
	signalProcessor->prepareBlocks(indexSet, 0);

	int size = lastIndex - firstIndex + 1;
	set<int> nextIndexSet = createSetFromRange(firstIndex + size, lastIndex + size);
	signalProcessor->prepareBlocks(nextIndexSet, 2);

	// Render one block at a time.
	while (indexSet.empty() == false)
	{
		SignalBlock block = signalProcessor->getAnyBlock(indexSet);

		paintBlock(block);

		indexSet.erase(block.getIndex());
		signalProcessor->release(block, 1);

		cerr << "Block " << block.getIndex() << " painted." << endl;
		checkGLMessages();
	}

	// Finish rendering.
	fun()->glFlush();
	//fun()->glFinish();

	fun()->glBindVertexArray(0);

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
	fun()->glBindVertexArray(block.geGLVertexArray());

	for (unsigned int i = 0; i < block.getchannelCount(); ++i)
	{
		paintChannel(i, block);
	}
}

void Canvas::paintChannel(unsigned int channel, const SignalBlock& block)
{
	// Set the uniform variables specific for every channel. (This could be moved to paintGL(), calculate all values at once and here update only index to arrays.)
	GLuint location = fun()->glGetUniformLocation(program->getGLProgram(), "y0");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float y0 = (channel + 0.5f)*height()/block.getchannelCount();
	fun()->glUniform1f(location, y0);

	location = fun()->glGetUniformLocation(program->getGLProgram(), "yScale");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	fun()->glUniform1f(location, -0.000008f*height());

	GLint size = block.getLastSample() - block.getFirstSample() + 1;
	GLint first = channel*size;

	location = fun()->glGetUniformLocation(program->getGLProgram(), "bufferOffset");
	checkNotErrorCode(location,static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	fun()->glUniform1i(location, block.getFirstSample() - first);

	// Draw onto the screen.
	fun();
	fun()->glDrawArrays(GL_LINE_STRIP, first, size);
	fun();
}

#undef fun
