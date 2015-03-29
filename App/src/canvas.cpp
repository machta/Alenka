#include "canvas.h"

#include "options.h"
#include "signalviewer.h"

#include <QMatrix4x4>

#include <cstdio>
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

	gl();
}

void Canvas::changeFile(DataFile* file)
{
	makeCurrent();

	assert(signalProcessor != nullptr);

	signalProcessor->changeFile(file);

	if (file != nullptr)
	{
		samplesRecorded = file->getSamplesRecorded();

		montageTable = file->getMontageTables()->front();
		eventTable = montageTable->getEventTable();
		eventTypeTable = file->getEventTypeTable();
	}

	this->file = file;

	doneCurrent();
}

void Canvas::initializeGL()
{
	if (PROGRAM_OPTIONS.isSet("glInfo"))
	{
		cout << "Version: " << gl()->glGetString(GL_VERSION) << endl;
		cout << "Renderer: " << gl()->glGetString(GL_RENDERER) << endl;
		cout << "Vendor: " << gl()->glGetString(GL_VENDOR) << endl;
		cout << "GLSH version: " << gl()->glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

		const GLubyte* str = gl()->glGetString(GL_EXTENSIONS);
		if (str == nullptr)
		{
			cout << "Extensions:" << endl;
		}
		else
		{
			cout << "Extensions: " << str << endl;
		}

		exit(EXIT_SUCCESS);
	}

	signalProcessor = new SignalProcessor;

	FILE* file1 = fopen(PROGRAM_OPTIONS["vert"].as<string>().c_str(), "rb");
	checkNotErrorCode(file1, nullptr, "File '" << PROGRAM_OPTIONS["vert"].as<string>() << "' could not be opened.");

	FILE* file2 = fopen(PROGRAM_OPTIONS["frag"].as<string>().c_str(), "rb");
	checkNotErrorCode(file2, nullptr, "File '" << PROGRAM_OPTIONS["frag"].as<string>() << "' could not be opened.");

	program = new OpenGLProgram(file1, file2);

	fclose(file1);
	fclose(file2);

	gl()->glUseProgram(program->getGLProgram());

	gl()->glEnable(GL_BLEND);
	gl()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl()->glClearColor(1, 1, 1, 0);

	checkGLMessages();
}

void Canvas::resizeGL(int /*w*/, int /*h*/)
{
	//checkGLMessages();
}

void Canvas::paintGL()
{
	logToFile("Painting started.");

	gl()->glClear(GL_COLOR_BUFFER_BIT);

	if (signalProcessor->ready())
	{
		// Calculate the transformMatrix.
		const SignalViewer* parent = reinterpret_cast<SignalViewer*>(parentWidget());
		double ratio = samplesRecorded/parent->getVirtualWidth();

		QRectF rect(parent->getPosition()*ratio, 0, width()*ratio, height());

		QMatrix4x4 matrix;
		matrix.ortho(rect);

		GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "transformMatrix");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

		// Create the data block range needed.
		int firstIndex = static_cast<unsigned int>(floor(parent->getPosition()*ratio));
		int lastIndex = static_cast<unsigned int>(ceil((parent->getPosition()+width())*ratio));

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
			// Enqueue all blocks.
			//prepareBlocks(firstIndex, lastIndex);
		}

		// Render one block at a time.
		while (indexSet.empty() == false)
		{
			SignalBlock block = signalProcessor->getAnyBlock(indexSet);

			drawBlock(block);

			gl()->glFlush();

			indexSet.erase(block.getIndex());

			//logToFile("Block " << block.getIndex() << " painted.");
		}

		// Finish rendering.
		gl()->glFlush();

		// Prepare some blocks for the next frame.
		int cap = min(PROGRAM_OPTIONS["prepareFrames"].as<unsigned int>()*indexSetSize, signalProcessor->getCapacity() - indexSetSize);

		prepare(indexSetSize, 0, static_cast<int>(ceil(parent->getVirtualWidth()*ratio)), firstIndex - indexSetSize, lastIndex + indexSetSize, cap);

		gl()->glBindVertexArray(0);
	}

	checkGLMessages();

	logToFile("Painting finished.");
}

void Canvas::drawBlock(const SignalBlock& block)
{
	gl()->glBindVertexArray(block.getGLVertexArray());

	// Paint events.
	GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "eventWidth");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1f(location, 1*height()/signalProcessor->getTrackCount());

	// TODO: Build a tree structure from the events to avoid the expensive enner loop.
	for (int type = 0; type < eventTypeTable->rowCount(); ++type)
	{
		setUniformColor(eventTypeTable->data(eventTypeTable->index(type, 3)).value<QColor>(), eventTypeTable->data(eventTypeTable->index(type, 2)).toDouble());

		for (int channel = 0; channel < signalProcessor->getTrackCount(); ++channel)
		{
			setUniformChannel(channel, block);

			for (int event = 0; event < eventTable->rowCount(); ++event)
			{
				int from = eventTable->data(eventTable->index(event, 2)).toInt() - block.getFirstSample();

				if (type == eventTable->data(eventTable->index(event, 1)).toInt()
					&& channel == eventTable->data(eventTable->index(event, 4)).toInt()
					&& from >= 0 && from < signalProcessor->getBlockSize())
				{
					int length = eventTable->data(eventTable->index(event, 3)).toInt();
					length = max(0, min<int>(signalProcessor->getBlockSize() - from, length));

					gl()->glDrawArrays(GL_TRIANGLE_STRIP, channel*signalProcessor->getBlockSize() + from, length);
				}
			}
		}
	}

	// Paint signals.
	location = gl()->glGetUniformLocation(program->getGLProgram(), "eventWidth");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1f(location, 0);

	for (unsigned int i = 0; i < signalProcessor->getTrackCount(); ++i)
	{
		setUniformChannel(i, block);
		setUniformColor(montageTable->data(montageTable->index(i, 3)).value<QColor>(), 1);

		gl()->glDrawArrays(GL_LINE_STRIP, i*signalProcessor->getBlockSize(), signalProcessor->getBlockSize());
	}
}

void Canvas::setUniformChannel(int channel, const SignalBlock& block)
{
	GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "y0");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float y0 = (channel + 0.5f)*height()/signalProcessor->getTrackCount();
	gl()->glUniform1f(location, y0);

	location = gl()->glGetUniformLocation(program->getGLProgram(), "yScale");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float yScale = montageTable->data(montageTable->index(channel, 3)).toDouble();
	gl()->glUniform1f(location, yScale*height());

	location = gl()->glGetUniformLocation(program->getGLProgram(), "bufferOffset");
	checkNotErrorCode(location,static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1i(location, block.getFirstSample() - channel*signalProcessor->getBlockSize());
}

void Canvas::setUniformColor(const QColor& color, float opacity)
{
	double r, g, b, a;
	color.getRgbF(&r, &g, &b, &a);
	a = opacity;

	GLuint location = gl()->glGetUniformLocation(program->getGLProgram(), "color");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform4f(location, r, g, b, a);
}
