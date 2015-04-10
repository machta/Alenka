#include "canvas.h"

#include "options.h"
#include "signalviewer.h"
#include "SignalProcessor/filter.h"

#include <QMatrix4x4>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor>

#include <cstdio>
#include <string>
#include <cmath>
#include <set>

using namespace std;

namespace
{
const double horizontalZoomFactor = 1.3;
const double verticalZoomFactor = 1.3;
const double trackZoomFactor = 1.3;
}

Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent)
{
	setFocusPolicy(Qt::ClickFocus);
	setMouseTracking(true);
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

		drawAllChannelEvents(allChannelEvents);

		// Draw one block at a time.
		while (indexSet.empty() == false)
		{
			SignalBlock block = signalProcessor->getAnyBlock(indexSet);

			drawSingleChannelEvents(block, singleChannelEvents);
			drawSignal(block);

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

void Canvas::wheelEvent(QWheelEvent* event)
{
	bool isUp;

	if (event->angleDelta().isNull() == false)
	{
		isUp = event->angleDelta().x() + event->angleDelta().y() > 0;
	}
	else if (event->pixelDelta().isNull() == false)
	{
		isUp = event->pixelDelta().x() + event->pixelDelta().y() > 0;
	}
	else
	{
		QOpenGLWidget::wheelEvent(event);
		return;
	}

	if (event->modifiers() & Qt::ControlModifier)
	{
		if (isUp)
		{
			trackZoom(trackZoomFactor);
		}
		else
		{
			trackZoom(1/trackZoomFactor);
		}
	}
	else if (event->modifiers() & Qt::AltModifier)
	{
		if (isUp)
		{
			horizontalZoom(horizontalZoomFactor);
		}
		else
		{
			horizontalZoom(1/horizontalZoomFactor);
		}
	}
	else if (event->modifiers() & Qt::ShiftModifier)
	{
		if (isUp)
		{
			verticalZoom(verticalZoomFactor);
		}
		else
		{
			verticalZoom(1/verticalZoomFactor);
		}
	}
	else
	{
		QOpenGLWidget::wheelEvent(event);
	}
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control)
	{
		if (signalProcessor->ready())
		{
			updateSelectedTrack();
			update();
		}

		if (isDrawingEvent == false)
		{
			isTrackSelected = true;
		}
	}
	else
	{
		QOpenGLWidget::keyPressEvent(event);
	}
}

void Canvas::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control)
	{
		isTrackSelected = false;
		//drawingEvent = false;

		update();
	}
//	else if (event->key() == Qt::Key_Shift)
//	{
//		drawingEvent = false;

//		update();
//	}
	else
	{
		QOpenGLWidget::keyReleaseEvent(event);
	}
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
	bool callBase = true;

	if (isTrackSelected)
	{
		updateSelectedTrack();
		update();

		callBase = false;
	}

	if (isDrawingEvent)
	{
		double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
		eventEnd = (event->pos().x() + getInfoTable()->getPosition())*ratio;

		update();

		callBase = false;
	}

	if (callBase)
	{
		QOpenGLWidget::mouseMoveEvent(event);
	}
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (event->modifiers() == Qt::ShiftModifier || event->modifiers() == Qt::ControlModifier)
		{
			if (signalProcessor->ready())
			{
				isDrawingEvent = true;

				double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
				eventStart = eventEnd = (event->pos().x() + getInfoTable()->getPosition())*ratio;
			}

			if (event->modifiers() == Qt::ShiftModifier)
			{
				selectedTrack = -1;
			}
			else if (event->modifiers() == Qt::ControlModifier)
			{
				updateSelectedTrack();
			}

			update();
		}
	}
	else
	{
		QOpenGLWidget::mousePressEvent(event);
	}
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (isDrawingEvent)
		{
			isDrawingEvent = false;

			addEvent(selectedTrack);
		}

		update();
	}
	else
	{
		QOpenGLWidget::mouseReleaseEvent(event);
	}
}

void Canvas::focusOutEvent(QFocusEvent* /*event*/)
{
	if (isTrackSelected)
	{
		isTrackSelected = false;
	}

	if (isDrawingEvent)
	{
		isDrawingEvent = false;
	}

	update();
}

void Canvas::drawAllChannelEvents(const std::vector<std::tuple<int, int, int>>& eventVector)
{
	gl()->glUseProgram(rectangleProgram->getGLProgram());
	gl()->glBindVertexArray(rectangleArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleBuffer);

	int event = 0, type = -1;
	while (event < static_cast<int>(eventVector.size()))
	{
		if (type == get<0>(eventVector[event]))
		{
			int from = get<1>(eventVector[event]);
			int to = from + get<2>(eventVector[event]) - 1;

			drawAllChannelEvent(from, to);

			++event;
		}
		else
		{
			type = get<0>(eventVector[event]);
			setUniformColor(rectangleProgram->getGLProgram(), eventTypeTable->getColor(type), eventTypeTable->getOpacity(type));
		}
	}

	if (isDrawingEvent)
	{
		if (selectedTrack == -1)
		{
			setUniformColor(rectangleProgram->getGLProgram(), QColor(Qt::blue), 0.5);

			int start = eventStart, end = eventEnd;

			if (end < start)
			{
				swap(start, end);
			}

			drawAllChannelEvent(start, end);
		}
	}
}

void Canvas::drawAllChannelEvent(int from, int to)
{
	float data[8] = {static_cast<float>(from), 0, static_cast<float>(to), 0, static_cast<float>(from), static_cast<float>(height()), static_cast<float>(to), static_cast<float>(height())};
	gl()->glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), data, GL_STATIC_DRAW);

	gl()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::drawSingleChannelEvents(const SignalBlock& block, const vector<tuple<int, int, int, int>>& eventVector)
{
	gl()->glUseProgram(eventProgram->getGLProgram());

	gl()->glBindVertexArray(block.getArray());

	int event = 0, type = -1, track = -1, hidden = 0;
	while (event < static_cast<int>(eventVector.size()))
	{
		if (type == get<0>(eventVector[event]))
		{
			if (track == get<1>(eventVector[event]))
			{
				int from = get<2>(eventVector[event]);
				int to = from + get<3>(eventVector[event]) - 1;

				drawSingleChannelEvent(block, track - hidden, from, to);

				++event;
			}
			else
			{
				track = get<1>(eventVector[event]);				
				assert(currentTrackTable()->getHidden(track) == false);

				hidden = 0;
				for (int i = 0; i < track; ++i)
				{
					if (currentTrackTable()->getHidden(i))
					{
						++hidden;
					}
				}

				setUniformTrack(eventProgram->getGLProgram(), track, hidden, block);
			}
		}
		else
		{
			type = get<0>(eventVector[event]);
			setUniformColor(eventProgram->getGLProgram(), eventTypeTable->getColor(type), eventTypeTable->getOpacity(type));
		}
	}

	if (isDrawingEvent)
	{
		int track = selectedTrack;

		if (0 <= track && track < signalProcessor->getTrackCount())
		{
			int hidden = countHiddenTracks(track);

			setUniformTrack(eventProgram->getGLProgram(), track + hidden, hidden, block);

			setUniformColor(eventProgram->getGLProgram(), QColor(Qt::blue), 0.5);

			int start = eventStart, end = eventEnd;

			if (end < start)
			{
				swap(start, end);
			}

			drawSingleChannelEvent(block, track, start, end);
		}
	}
}

void Canvas::drawSingleChannelEvent(const SignalBlock& block, int track, int from, int to)
{
	if (from <= block.getLastSample() && block.getFirstSample() <= to)
	{
		from = max<int>(block.getFirstSample(), from);
		to = min<int>(block.getLastSample(), to);

		if (eventMode == 1)
		{
			gl()->glDrawArrays(GL_TRIANGLE_STRIP, track*signalProcessor->getBlockSize() + from - block.getFirstSample(), to - from + 1);
		}
		else
		{
			gl()->glDrawArrays(GL_TRIANGLE_STRIP, 2*(track*signalProcessor->getBlockSize() + from - block.getFirstSample()), 2*(to - from + 1));
		}
	}
}

void Canvas::drawSignal(const SignalBlock& block)
{
	gl()->glUseProgram(signalProgram->getGLProgram());

	if (eventMode == 1)
	{
		gl()->glBindVertexArray(block.getArray());
	}
	else
	{
		gl()->glBindVertexArray(block.getArrayStrideTwo());
	}

	for (int track = 0; track < signalProcessor->getTrackCount(); ++track)
	{
		int hidden = countHiddenTracks(track);

		setUniformTrack(signalProgram->getGLProgram(), track + hidden, hidden, block);

		QColor color = currentTrackTable()->getColor(track + hidden);

		if (isTrackSelected && isDrawingEvent == false && track == selectedTrack)
		{
			double colorComponents[3] = {color.redF(), color.greenF(), color.blueF()};

			for (int i = 0; i < 3; ++i)
			{
				colorComponents[i] += colorComponents[i] > 0.5 ? -0.45 : 0.45;
			}

			color.setRedF(colorComponents[0]);
			color.setGreenF(colorComponents[1]);
			color.setBlueF(colorComponents[2]);
		}

		setUniformColor(signalProgram->getGLProgram(), color, 1);

		gl()->glDrawArrays(GL_LINE_STRIP, track*signalProcessor->getBlockSize(), signalProcessor->getBlockSize());
	}
}

void Canvas::setUniformTrack(GLuint program, int track, int hidden, const SignalBlock& block)
{
	GLuint location = gl()->glGetUniformLocation(program, "y0");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float y0 = (track - hidden + 0.5f)*height()/signalProcessor->getTrackCount();
	gl()->glUniform1f(location, y0);

	location = gl()->glGetUniformLocation(program, "yScale");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	float yScale = currentTrackTable()->getAmplitude(track);
	gl()->glUniform1f(location, yScale*height());

	location = gl()->glGetUniformLocation(program, "bufferOffset");
	checkNotErrorCode(location,static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform1i(location, block.getFirstSample() - (track - hidden)*signalProcessor->getBlockSize());
}

void Canvas::setUniformColor(GLuint program, const QColor& color, double opacity)
{
	double r, g, b, a;
	color.getRgbF(&r, &g, &b, &a);
	a = opacity;

	GLuint location = gl()->glGetUniformLocation(program, "color");
	checkNotErrorCode(location, static_cast<GLuint>(-1), "glGetUniformLocation() failed.");
	gl()->glUniform4f(location, r, g, b, a);
}

void Canvas::horizontalZoom(double factor)
{
	InfoTable* it = getInfoTable();
	it->setVirtualWidth(it->getVirtualWidth()*factor);
}

void Canvas::verticalZoom(double factor)
{
	if (montageTable != nullptr)
	{
		TrackTable* tt = montageTable->getTrackTables()->at(getInfoTable()->getSelectedMontage());

		for (int i = 0; i < tt->rowCount(); ++i)
		{
			tt->setAmplitude(tt->getAmplitude(i)*factor, i);
		}

		emit tt->dataChanged(tt->index(0, static_cast<int>(TrackTable::Column::amplitude)), tt->index(tt->rowCount() - 1, static_cast<int>(TrackTable::Column::amplitude)));
	}
}

void Canvas::trackZoom(double factor)
{
	if (montageTable != nullptr)
	{
		TrackTable* tt = montageTable->getTrackTables()->at(getInfoTable()->getSelectedMontage());

		int track = selectedTrack;

		track += countHiddenTracks(track);

		if (0 <= track && track < tt->rowCount())
		{
			tt->setAmplitude(tt->getAmplitude(track)*factor, track);
		}

		emit tt->dataChanged(tt->index(0, static_cast<int>(TrackTable::Column::amplitude)), tt->index(tt->rowCount() - 1, static_cast<int>(TrackTable::Column::amplitude)));
	}
}

void Canvas::updateSelectedTrack()
{
	int y = mapFromGlobal(QCursor::pos()).y();

	double trackHeigth = static_cast<double>(height())/signalProcessor->getTrackCount();
	int track = min(static_cast<int>(y/trackHeigth), signalProcessor->getTrackCount() - 1);

	selectedTrack = track;
}

void Canvas::addEvent(int channel)
{
	EventTable* et = montageTable->getEventTables()->at(getInfoTable()->getSelectedMontage());

	int index = et->rowCount();
	et->insertRowsBack();

	et->setChannel(channel + countHiddenTracks(channel), index);
	if (eventEnd < eventStart)
	{
		swap(eventStart, eventEnd);
	}
	et->setPosition(eventStart, index);
	et->setDuration(eventEnd - eventStart + 1, index);
}

int Canvas::countHiddenTracks(int track)
{
	int hidden = 0;
	int i = 0;
	while (i - hidden <= track)
	{
		if (currentTrackTable()->getHidden(i))
		{
			++hidden;
		}

		++i;
	}

	return hidden;
}
