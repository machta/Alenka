#include "canvas.h"

#include "options.h"
#include "signalviewer.h"
#include "SignalProcessor/filter.h"

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
	delete signalProgram;
	delete eventProgram;
	delete rectangleProgram;

	makeCurrent();

	gl()->glDeleteVertexArrays(1, &rectangleArray);
	gl()->glDeleteBuffers(1, &rectangleBuffer);

	gl();

	doneCurrent();
}

void Canvas::changeFile(DataFile* file)
{
	assert(signalProcessor != nullptr);

	makeCurrent();

	signalProcessor->changeFile(file);

	if (file == nullptr)
	{
		infoTable = nullptr;
		montageTable = nullptr;
		eventTypeTable = nullptr;
	}
	else
	{
		infoTable = file->getInfoTable();
		montageTable = file->getMontageTable();
		eventTypeTable = file->getEventTypeTable();

		connect(infoTable, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(updateFilter()));
		connect(infoTable, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(updateFilter()));
		connect(infoTable, SIGNAL(notchChanged(bool)), this, SLOT(updateFilter()));

		connect(infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(selectMontage()));

		samplesRecorded = file->getSamplesRecorded();
	}

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

	FILE* signalVert = fopen("signal.vert", "rb");
	checkNotErrorCode(signalVert, nullptr, "File 'signal.vert' could not be opened.");

	FILE* eventVert = fopen("event.vert", "rb");
	checkNotErrorCode(eventVert, nullptr, "File 'event.vert' could not be opened.");

	FILE* rectangleVert = fopen("rectangle.vert", "rb");
	checkNotErrorCode(rectangleVert, nullptr, "File 'rectangle.vert' could not be opened.");

	FILE* colorFrag = fopen("color.frag", "rb");
	checkNotErrorCode(colorFrag, nullptr, "File 'color.frag could' not be opened.");

	signalProgram = new OpenGLProgram(signalVert, colorFrag);
	eventProgram = new OpenGLProgram(eventVert, colorFrag);
	rectangleProgram = new OpenGLProgram(rectangleVert, colorFrag);

	fclose(signalVert);
	fclose(eventVert);
	fclose(rectangleVert);
	fclose(colorFrag);

	gl()->glEnable(GL_BLEND);
	gl()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl()->glClearColor(1, 1, 1, 0);

	gl()->glGenVertexArrays(1, &rectangleArray);
	gl()->glGenBuffers(1, &rectangleBuffer);

	gl()->glBindVertexArray(rectangleArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleBuffer);
	gl()->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl()->glBindVertexArray(0);

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
		double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();

		QMatrix4x4 matrix;
		matrix.ortho(QRectF(getInfoTable()->getPosition()*ratio, 0, width()*ratio, height()));

		gl()->glUseProgram(signalProgram->getGLProgram());

		GLuint location = gl()->glGetUniformLocation(signalProgram->getGLProgram(), "transformMatrix");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

		gl()->glUseProgram(eventProgram->getGLProgram());

		location = gl()->glGetUniformLocation(eventProgram->getGLProgram(), "transformMatrix");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

		location = gl()->glGetUniformLocation(eventProgram->getGLProgram(), "divideBy");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniform1i(location, eventMode);

		location = gl()->glGetUniformLocation(eventProgram->getGLProgram(), "eventWidth");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniform1f(location, 0.45*height()/signalProcessor->getTrackCount());

		gl()->glUseProgram(rectangleProgram->getGLProgram());

		location = gl()->glGetUniformLocation(rectangleProgram->getGLProgram(), "transformMatrix");
		checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
		gl()->glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());

		// Create the data block range needed.
		int firstSample = static_cast<unsigned int>(floor(getInfoTable()->getPosition()*ratio));
		int lastSample = static_cast<unsigned int>(ceil((getInfoTable()->getPosition() + width())*ratio));

		auto fromTo = DataFile::sampleRangeToBlockRange(make_pair(firstSample, lastSample), signalProcessor->getBlockSize());

		set<int> indexSet;

		for (int i = fromTo.first; i <= fromTo.second; ++i)
		{
			indexSet.insert(i);
		}

		if (signalProcessor->getCapacity() > indexSet.size())
		{
			// Enqueue all blocks.
			//prepareBlocks(firstIndex, lastIndex);
		}

		// Get events.
		vector<tuple<int, int, int>> allChannelEvents;
		vector<tuple<int, int, int, int>> singleChannelEvents;

		currentEventTable()->getEventsForRendering(firstSample, lastSample, &allChannelEvents, &singleChannelEvents);

		// Draw all-channel events.
		gl()->glUseProgram(rectangleProgram->getGLProgram());
		gl()->glBindVertexArray(rectangleArray);
		gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleBuffer);

		int event = 0, type = -1;
		while (event < static_cast<int>(allChannelEvents.size()))
		{
			if (type == get<0>(allChannelEvents[event]))
			{
				int from = get<1>(allChannelEvents[event]);
				int to = from + get<2>(allChannelEvents[event]) - 1;

				float data[8] = {static_cast<float>(from), 0, static_cast<float>(to), 0, static_cast<float>(from), static_cast<float>(height()), static_cast<float>(to), static_cast<float>(height())};
				gl()->glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), data, GL_STATIC_DRAW);

				gl()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				++event;
			}
			else
			{
				type = get<0>(allChannelEvents[event]);
				setUniformColor(rectangleProgram->getGLProgram(), eventTypeTable->data(eventTypeTable->index(type, 3)).value<QColor>(), eventTypeTable->data(eventTypeTable->index(type, 2)).toDouble());
			}
		}

		// Draw one block at a time.
		while (indexSet.empty() == false)
		{
			SignalBlock block = signalProcessor->getAnyBlock(indexSet);

			drawBlock(block, singleChannelEvents);

			gl()->glFlush();

			indexSet.erase(block.getIndex());

			//logToFile("Block " << block.getIndex() << " painted.");
		}

		gl()->glBindVertexArray(0);

		// Prepare some blocks for the next frame.
		int cap = min(PROGRAM_OPTIONS["prepareFrames"].as<unsigned int>()*indexSet.size(), signalProcessor->getCapacity() - indexSet.size());

		prepare(indexSet.size(), 0, static_cast<int>(ceil(getInfoTable()->getVirtualWidth()*ratio)), fromTo.first - indexSet.size(), fromTo.second + indexSet.size(), cap);
	}

	gl()->glFinish();

	checkGLMessages();

	logToFile("Painting finished.");
}

void Canvas::drawBlock(const SignalBlock& block, const vector<tuple<int, int, int, int>>& singleChannelEvents)
{
	// Draw single-channel events.
	gl()->glUseProgram(eventProgram->getGLProgram());

	gl()->glBindVertexArray(block.getArray());

	int event = 0, type = -1, channel = signalProcessor->getTrackCount();
	while (event < static_cast<int>(singleChannelEvents.size()))
	{
		if (type == get<0>(singleChannelEvents[event]))
		{
			if (channel == get<1>(singleChannelEvents[event]))
			{
				int from = get<2>(singleChannelEvents[event]);
				int to = from + get<3>(singleChannelEvents[event]) - 1;

				if (from <= block.getLastSample() && block.getFirstSample() <= to)
				{
					from = max<int>(block.getFirstSample(), from);
					to = min<int>(block.getLastSample(), to);

					if (eventMode == 1)
					{
						gl()->glDrawArrays(GL_TRIANGLE_STRIP, channel*signalProcessor->getBlockSize() + from - block.getFirstSample(), to - from + 1);
					}
					else
					{
						gl()->glDrawArrays(GL_TRIANGLE_STRIP, 2*(channel*signalProcessor->getBlockSize() + from - block.getFirstSample()), 2*(to - from + 1));
					}
				}

				++event;
			}
			else
			{
				channel = get<1>(singleChannelEvents[event]);
				setUniformChannel(eventProgram->getGLProgram(), channel, block);
			}
		}
		else
		{
			type = get<0>(singleChannelEvents[event]);
			setUniformColor(eventProgram->getGLProgram(), eventTypeTable->data(eventTypeTable->index(type, 3)).value<QColor>(), eventTypeTable->data(eventTypeTable->index(type, 2)).toDouble());
		}
	}

	// Draw signal.
	gl()->glUseProgram(signalProgram->getGLProgram());

	if (eventMode == 1)
	{
		gl()->glBindVertexArray(block.getArray());
	}
	else
	{
		gl()->glBindVertexArray(block.getArrayStrideTwo());
	}

	for (int i = 0; i < signalProcessor->getTrackCount(); ++i)
	{
		setUniformChannel(signalProgram->getGLProgram(), i, block);
		setUniformColor(signalProgram->getGLProgram(), currentTrackTable()->data(currentTrackTable()->index(i, 3)).value<QColor>(), 1);

		gl()->glDrawArrays(GL_LINE_STRIP, i*signalProcessor->getBlockSize(), signalProcessor->getBlockSize());
	}
}

void Canvas::setUniformChannel(GLuint program, int channel, const SignalBlock& block)
{
	GLuint location = gl()->glGetUniformLocation(program, "y0");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float y0 = (channel + 0.5f)*height()/signalProcessor->getTrackCount();
	gl()->glUniform1f(location, y0);

	location = gl()->glGetUniformLocation(program, "yScale");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float yScale = currentTrackTable()->data(currentTrackTable()->index(channel, 3)).toDouble();
	gl()->glUniform1f(location, yScale*height());

	location = gl()->glGetUniformLocation(program, "bufferOffset");
	checkNotErrorCode(location,static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1i(location, block.getFirstSample() - channel*signalProcessor->getBlockSize());
}

void Canvas::setUniformColor(GLuint program, const QColor& color, float opacity)
{
	double r, g, b, a;
	color.getRgbF(&r, &g, &b, &a);
	a = opacity;

	GLuint location = gl()->glGetUniformLocation(program, "color");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform4f(location, r, g, b, a);
}
