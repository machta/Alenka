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

	gl();
}

void Canvas::initializeGL()
{
	if (PROGRAM_OPTIONS.isSet("glInfo"))
	{
		cout << "Version: " << gl()->glGetString(GL_VERSION) << endl;
		cout << "Renderer: " << gl()->glGetString(GL_RENDERER) << endl;
		cout << "Vendor: " << gl()->glGetString(GL_VENDOR) << endl;
		cout << "GLSH version: " << gl()->glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
		cout << "Extensions: " << gl()->glGetString(GL_EXTENSIONS) << endl;

		exit(EXIT_SUCCESS);
	}

	dataFile = new GDF2(PROGRAM_OPTIONS["file"].as<string>().c_str());

	signalProcessor = new SignalProcessor(dataFile);

	FILE* file1 = fopen(PROGRAM_OPTIONS["vert"].as<string>().c_str(), "rb");
	checkNotErrorCode(file1, nullptr, "File '" << PROGRAM_OPTIONS["vert"].as<string>() << "' could not be opened.");

	FILE* file2 = fopen(PROGRAM_OPTIONS["frag"].as<string>().c_str(), "rb");
	checkNotErrorCode(file2, nullptr, "File '" << PROGRAM_OPTIONS["frag"].as<string>() << "' could not be opened.");

	program = new OpenGLProgram(file1, file2);

	fclose(file1);
	fclose(file2);

	gl()->glUseProgram(program->getGLProgram());

	gl()->glClearColor(1, 1, 1, 1);

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
	gl()->glClear(GL_COLOR_BUFFER_BIT);

	// Calculate the transformMatrix.
	const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());
	double ratio = samplePixelRatio();

	QRectF rect(parent->getPosition()*ratio, 0, width()*ratio, height());

	QMatrix4x4 matrix;
	matrix.ortho(rect);

	GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "transformMatrix");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

	// Create the data block range needed.
	int firstIndex = static_cast<unsigned int>(floor(parent->getPosition()*ratio)),
		lastIndex = static_cast<unsigned int>(ceil((parent->getPosition()+width())*ratio));

	firstIndex /= signalProcessor->getBlockSize();
	lastIndex /= signalProcessor->getBlockSize();

	set<int> indexSet;

	unsigned int indexSetSize = lastIndex - firstIndex + 1;
	for (unsigned int i = 0; i < indexSetSize; ++i)
	{
		indexSet.insert(firstIndex + i);
	}

	if (signalProcessor->getCapacity() > indexSetSize)
	{
		//prepareBlocks(firstIndex, lastIndex);
	}

	// Render one block at a time.
	while (indexSet.empty() == false)
	{
		SignalBlock block = signalProcessor->getAnyBlock(indexSet);

		paintBlock(block);

		gl()->glFlush();

		indexSet.erase(block.getIndex());

		cerr << "Block " << block.getIndex() << " painted." << endl;
	}

	// Finish rendering.
	gl()->glFlush();
	//fun()->glFinish();

	// Prepare some blocks for next frame.
	//prepare(indexSetSize, 0, static_cast<int>(ceil(parent->getVirtualWidth()*ratio)), firstIndex - indexSetSize, lastIndex + indexSetSize, min(4*indexSetSize, signalProcessor->getCapacity() - indexSetSize));

	gl()->glBindVertexArray(0);

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
	gl()->glBindVertexArray(block.getGLVertexArray());

	for (unsigned int i = 0; i < block.getchannelCount(); ++i)
	{
		paintChannel(i, block);
	}
}

void Canvas::paintChannel(unsigned int channel, const SignalBlock& block)
{
	// Set the uniform variables specific for every channel. (This could be moved to paintGL(), calculate all values at once and here update only index to arrays.)
	GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "y0");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float y0 = (channel + 0.5f)*height()/block.getchannelCount();
	gl()->glUniform1f(location, y0);

	location = gl()->glGetUniformLocation(program->getGLProgram(), "yScale");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1f(location, -0.000008f*height());

	GLint size = block.getLastSample() - block.getFirstSample() + 1;
	GLint first = channel*size;

	location = gl()->glGetUniformLocation(program->getGLProgram(), "bufferOffset");
	checkNotErrorCode(location,static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1i(location, block.getFirstSample() - first);

	// Draw onto the screen.
	gl();
	gl()->glDrawArrays(GL_LINE_STRIP, first, size);
	gl();
}
