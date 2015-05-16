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
	delete rectangleLineProgram;

	makeCurrent();

	glDeleteVertexArrays(1, &rectangleLineArray);
	gl()->glDeleteBuffers(1, &rectangleLineBuffer);

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

		connect(infoTable, SIGNAL(positionChanged(int)), this, SLOT(updateCursor()));

		samplesRecorded = file->getSamplesRecorded();
		samplingFrequency = file->getSamplingFrequency();
	}

	doneCurrent();
}

QColor Canvas::modifySelectionColor(const QColor& color)
{
	double colorComponents[3] = {color.redF(), color.greenF(), color.blueF()};

	for (int i = 0; i < 3; ++i)
	{
		colorComponents[i] += colorComponents[i] > 0.5 ? -0.45 : 0.45;
	}

	QColor newColor(color);
	newColor.setRedF(colorComponents[0]);
	newColor.setGreenF(colorComponents[1]);
	newColor.setBlueF(colorComponents[2]);
	return newColor;
}

void Canvas::updateCursor()
{
	if (ready())
	{
		QPoint pos = mapFromGlobal(QCursor::pos());

		double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
		int sample = round((pos.x() + getInfoTable()->getPosition())*ratio);

		double trackHeigth = static_cast<double>(height())/signalProcessor->getTrackCount();
		int track = static_cast<int>(pos.y()/trackHeigth);

		setCursorPositionSample(sample);
		setCursorPositionTrack(track);

		if (isDrawingEvent)
		{
			eventEnd = sample;

			update();
		}
	}
}

void Canvas::initializeGL()
{
	logToFile("Initializing OpenGL in Canvas.");

	initializeOpenGLInterface();

	signalProcessor = new SignalProcessor;

	FILE* signalVert = fopen("signal.vert", "rb");
	checkNotErrorCode(signalVert, nullptr, "File 'signal.vert' could not be opened.");

	FILE* eventVert = fopen("event.vert", "rb");
	checkNotErrorCode(eventVert, nullptr, "File 'event.vert' could not be opened.");

	FILE* rectangleLineVert = fopen("rectangleLine.vert", "rb");
	checkNotErrorCode(rectangleLineVert, nullptr, "File 'rectangleLine.vert' could not be opened.");

	FILE* colorFrag = fopen("color.frag", "rb");
	checkNotErrorCode(colorFrag, nullptr, "File 'color.frag could' not be opened.");

	signalProgram = new OpenGLProgram(signalVert, colorFrag);
	eventProgram = new OpenGLProgram(eventVert, colorFrag);
	rectangleLineProgram = new OpenGLProgram(rectangleLineVert, colorFrag);

	int err = fclose(signalVert);
	checkErrorCode(err, 0, "fclose()");

	err = fclose(eventVert);
	checkErrorCode(err, 0, "fclose()");

	err = fclose(rectangleLineVert);
	checkErrorCode(err, 0, "fclose()");

	err = fclose(colorFrag);
	checkErrorCode(err, 0, "fclose()");

	gl()->glEnable(GL_BLEND);
	gl()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl()->glClearColor(1, 1, 1, 0);

	glGenVertexArrays(1, &rectangleLineArray);
	gl()->glGenBuffers(1, &rectangleLineBuffer);

	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);
	gl()->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Log OpenGL details.
	stringstream ss;

	ss << "OpenGL info:" << endl;
	ss << "Version: " << gl()->glGetString(GL_VERSION) << endl;
	ss << "Renderer: " << gl()->glGetString(GL_RENDERER) << endl;
	ss << "Vendor: " << gl()->glGetString(GL_VENDOR) << endl;
	ss << "GLSH version: " << gl()->glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

#ifdef GL_2_0
	const GLubyte* str = gl()->glGetString(GL_EXTENSIONS);
	if (str == nullptr)
	{
		ss << "Extensions:" << endl;
	}
	else
	{
		ss << "Extensions: " << str << endl;
	}
#else
	GLint extensions;
	gl()->glGetIntegerv(GL_NUM_EXTENSIONS, &extensions);

	ss << "Extensions:";
	for (GLint i = 0; i < extensions; ++i)
	{
		ss << " " << gl()->glGetStringi(GL_EXTENSIONS, i);
	}
	ss << endl;
#endif

	logToFile(ss.str());

	if (PROGRAM_OPTIONS.isSet("glInfo"))
	{
		cout << ss.str();
		std::exit(EXIT_SUCCESS);
	}

	checkGLMessages();
}

void Canvas::resizeGL(int /*w*/, int /*h*/)
{
	//checkGLMessages();
}

void Canvas::paintGL()
{
#ifndef NDEBUG
	logToFile("Painting started.");
#endif

	gl()->glClear(GL_COLOR_BUFFER_BIT);

	if (ready())
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

		gl()->glUseProgram(rectangleLineProgram->getGLProgram());

		location = gl()->glGetUniformLocation(rectangleLineProgram->getGLProgram(), "transformMatrix");
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

		// Get events.
		vector<tuple<int, int, int>> allChannelEvents;
		vector<tuple<int, int, int, int>> singleChannelEvents;
		currentEventTable()->getEventsForRendering(firstSample, lastSample, &allChannelEvents, &singleChannelEvents);

		// Draw.
		drawTimeLines();
		drawAllChannelEvents(allChannelEvents);

		while (indexSet.empty() == false)
		{
			SignalBlock block = signalProcessor->getAnyBlock(indexSet);

			drawSingleChannelEvents(block, singleChannelEvents);
			drawSignal(block);

			gl()->glFlush();

			indexSet.erase(block.getIndex());

			//logToFile("Block " << block.getIndex() << " painted.");
		}

		drawPositionIndicator();
		drawCross();

		glBindVertexArray(0);
	}

	gl()->glFinish();

	checkGLMessages();

#ifndef NDEBUG
	logToFile("Painting finished.");
#endif
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
		event->ignore();
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

		update();
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

		updateCursor();
		emit getInfoTable()->positionIndicatorChanged(getInfoTable()->getPositionIndicator());

		update();
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

		update();
	}
	else
	{
		event->ignore();
	}
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control)
	{
		if (isDrawingEvent == false)
		{
			isSelectingTrack = true;
			update();
		}
	}
	else if (event->key() == Qt::Key_C)
	{
		isDrawingCross = !isDrawingCross; // Perhaps promote this to an action?
		update();
	}
	else if (ready() && event->key() == Qt::Key_T)
	{
		QPoint pos = mapFromGlobal(QCursor::pos());
		double indicator = static_cast<double>(pos.x())/width();

		if (0 <= indicator && indicator <= 1)
		{
			getInfoTable()->setPositionIndicator(indicator);

			update();
		}
	}
	else
	{
		event->ignore();
	}
}

void Canvas::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control)
	{
		isSelectingTrack = false;

		update();
	}
}

void Canvas::mouseMoveEvent(QMouseEvent* /*event*/)
{
	if (ready())
	{
		updateCursor();

		if (isSelectingTrack || isDrawingEvent || isDrawingCross)
		{
			update();
		}
	}
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (event->modifiers() == Qt::ShiftModifier || event->modifiers() == Qt::ControlModifier)
		{
			if (ready() && 0 <= cursorTrack && cursorTrack < signalProcessor->getTrackCount())
			{
				isDrawingEvent = true;

				double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
				eventStart = eventEnd = (event->pos().x() + getInfoTable()->getPosition())*ratio;

				if (event->modifiers() == Qt::ShiftModifier)
				{
					eventTrack = -1;
				}
				else if (event->modifiers() == Qt::ControlModifier)
				{
					eventTrack = cursorTrack;
				}

				isSelectingTrack = false;

				update();
			}
		}
	}
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (isDrawingEvent)
		{
			isDrawingEvent = false;

			addEvent(eventTrack);

			update();
		}
	}
}

void Canvas::focusOutEvent(QFocusEvent* /*event*/)
{
	if (isSelectingTrack)
	{
		isSelectingTrack = false;
		update();
	}
	else if (isDrawingEvent)
	{
		isDrawingEvent = false;
		update();
	}
	else if (isDrawingCross)
	{
		update();
	}
}

void Canvas::focusInEvent(QFocusEvent* /*event*/)
{
}

void Canvas::drawAllChannelEvents(const std::vector<std::tuple<int, int, int>>& eventVector)
{
	gl()->glUseProgram(rectangleLineProgram->getGLProgram());
	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

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
			setUniformColor(rectangleLineProgram->getGLProgram(), eventTypeTable->getColor(type), eventTypeTable->getOpacity(type));
		}
	}

	if (isDrawingEvent)
	{
		if (eventTrack == -1)
		{
			QColor color(Qt::blue);
			double opacity = 0.5;
			int type = getInfoTable()->getSelectedType() - 1;
			if (type != -1)
			{
				color = eventTypeTable->getColor(type);
				opacity = eventTypeTable->getOpacity(type);
			}
			setUniformColor(rectangleLineProgram->getGLProgram(), color, opacity);

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

	gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	gl()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::drawTimeLines()
{
	double interval = getInfoTable()->getTimeLineInterval();

	gl()->glUseProgram(rectangleLineProgram->getGLProgram());
	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

	setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::green), 1);

	double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
	interval *= samplingFrequency;

	double position = getInfoTable()->getPosition()*ratio;
	double end = position + width()*ratio;

	int nextPosition = ceil(position/interval)*interval;

	if (interval > 0)
	{
		while (nextPosition <= end)
		{
			drawTimeLine(nextPosition);

			nextPosition += interval;
		}
	}
}

void Canvas::drawPositionIndicator()
{
	gl()->glUseProgram(rectangleLineProgram->getGLProgram());
	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

	setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::blue), 1);

	double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();
	double position = (getInfoTable()->getPosition() + width()*getInfoTable()->getPositionIndicator())*ratio;

	drawTimeLine(position);
}

void Canvas::drawCross()
{
	QPoint pos = mapFromGlobal(QCursor::pos());

	if (isDrawingCross == false || hasFocus() == false || pos.x() < 0 || pos.x() >= width() || pos.y() < 0 || pos.y() >= height())
	{
		return;
	}

	gl()->glUseProgram(rectangleLineProgram->getGLProgram());
	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

	setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::black), 1);

	double ratio = samplesRecorded/getInfoTable()->getVirtualWidth();

	double position = (getInfoTable()->getPosition() + pos.x())*ratio;

	float data[8] = {static_cast<float>(position), 0, static_cast<float>(position), static_cast<float>(height()), 0, static_cast<float>(pos.y()), static_cast<float>(samplesRecorded), static_cast<float>(pos.y())};

	gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	gl()->glDrawArrays(GL_LINE_STRIP, 0, 2);
	gl()->glDrawArrays(GL_LINE_STRIP, 2, 2);
}

void Canvas::drawTimeLine(double at)
{
	float data[4] = {static_cast<float>(at), 0, static_cast<float>(at), static_cast<float>(height())};

	gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	gl()->glDrawArrays(GL_LINE_STRIP, 0, 2);
}

void Canvas::drawSingleChannelEvents(const SignalBlock& block, const vector<tuple<int, int, int, int>>& eventVector)
{
	gl()->glUseProgram(eventProgram->getGLProgram());

	glBindVertexArray(block.getArray());

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
		int track = eventTrack;

		if (0 <= track && track < signalProcessor->getTrackCount())
		{
			int hidden = countHiddenTracks(track);

			setUniformTrack(eventProgram->getGLProgram(), track + hidden, hidden, block);

			QColor color(Qt::blue);
			double opacity = 0.5;
			int type = getInfoTable()->getSelectedType() - 1;
			if (type != -1)
			{
				color = eventTypeTable->getColor(type);
				opacity = eventTypeTable->getOpacity(type);
			}
			setUniformColor(eventProgram->getGLProgram(), color, opacity);

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
		glBindVertexArray(block.getArray());
	}
	else
	{
		glBindVertexArray(block.getArrayStrideTwo());
	}

	for (int track = 0; track < signalProcessor->getTrackCount(); ++track)
	{
		int hidden = countHiddenTracks(track);

		setUniformTrack(signalProgram->getGLProgram(), track + hidden, hidden, block);

		QColor color = currentTrackTable()->getColor(track + hidden);

		if (isSelectingTrack && track == cursorTrack)
		{
			color = modifySelectionColor(color);
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
			double value = tt->getAmplitude(i)*factor;
			value = value != 0 ? value : -0.000001;
			tt->setAmplitude(value, i);
		}

		emit tt->emitColumnChanged(TrackTable::Column::amplitude);
	}
}

void Canvas::trackZoom(double factor)
{
	if (montageTable != nullptr)
	{
		TrackTable* tt = montageTable->getTrackTables()->at(getInfoTable()->getSelectedMontage());

		int track = cursorTrack;

		track += countHiddenTracks(track);

		double value = tt->getAmplitude(track)*factor;
		value = value != 0 ? value : -0.000001;
		tt->setAmplitude(value, track);

		emit tt->emitColumnChanged(TrackTable::Column::amplitude);
	}
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

void Canvas::updateMontage(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (bottomRight.row() - topLeft.row() >= 0)
	{
		int column = static_cast<int>(TrackTable::Column::code);
		if (topLeft.column() <= column && column <= bottomRight.column())
		{
			updateMontage();
			return;
		}

		column = static_cast<int>(TrackTable::Column::hidden);
		if (topLeft.column() <= column && column <= bottomRight.column())
		{
			updateMontage();
			return;
		}
	}
}
