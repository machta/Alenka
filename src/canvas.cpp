#include "canvas.h"

#include "openglprogram.h"
#include "options.h"
#include "error.h"
#include <AlenkaFile/datafile.h>
#include "signalviewer.h"
#include "SignalProcessor/signalblock.h"
#include "SignalProcessor/signalprocessor.h"
#include "DataModel/opendatafile.h"
#include "DataModel/vitnessdatamodel.h"
#include "DataModel/undocommandfactory.h"

#include <QMatrix4x4>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QUndoCommand>

#include <cstdio>
#include <string>
#include <cmath>
#include <set>

using namespace std;
using namespace AlenkaFile;

namespace
{

const double horizontalZoomFactor = 1.3;
const double verticalZoomFactor = 1.3;
const double trackZoomFactor = verticalZoomFactor;

void getEventTypeColorOpacity(OpenDataFile* file, int type, QColor* color, double* opacity)
{
	assert(color && opacity);
	EventType et = file->dataModel->eventTypeTable()->row(type);
	*color = DataModel::array2color<QColor>(et.color);
	*opacity = et.opacity;
}

const AbstractTrackTable* getTrackTable(OpenDataFile* file)
{
	return file->dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
}

const AbstractEventTable* getEventTable(OpenDataFile* file)
{
	return file->dataModel->montageTable()->eventTable(OpenDataFile::infoTable.getSelectedMontage());
}

class ZoomCommand : public QUndoCommand
{
	DataModel* dataModel;
	int i, j;
	double before, after;
	const int commandId = 0;
	vector<QUndoCommand*> childCommands;

public:
	ZoomCommand(DataModel* dataModel, int i, int j, double before, double after) :
		QUndoCommand("vertical zoom"), dataModel(dataModel), i(i), j(j), before(before), after(after) {}
	virtual ~ZoomCommand() override
	{
		for (QUndoCommand* c : childCommands)
			delete c;
	}

	virtual void redo() override
	{
		Track t = dataModel->montageTable()->trackTable(i)->row(j);
		t.amplitude = after;
		dataModel->montageTable()->trackTable(i)->row(j, t);

		for (QUndoCommand* c : childCommands)
			c->redo();
	}
	virtual void undo() override
	{
		for (auto i = childCommands.rbegin(); i != childCommands.rend(); ++i)
			(*i)->undo();

		Track t = dataModel->montageTable()->trackTable(i)->row(j);
		t.amplitude = before;
		dataModel->montageTable()->trackTable(i)->row(j, t);
	}

	virtual int id() const override
	{
		return commandId;
	}
	virtual bool mergeWith(const QUndoCommand* other) override
	{
		assert(other->id() == commandId);

		auto o = dynamic_cast<const ZoomCommand*>(other);
		childCommands.push_back(new ZoomCommand(o->dataModel, o->i, o->j, o->before, o->after));

		return true;
	}
};

void zoom(OpenDataFile* file, double factor, int i)
{
	Track track = getTrackTable(file)->row(i);

	const double before = track.amplitude;

	double after = before*factor;
	after = after != 0 ? after : -0.000001;

	file->undoFactory->push(new ZoomCommand(file->file->getDataModel(), OpenDataFile::infoTable.getSelectedMontage(), i, before, after));
}

/**
 * @brief A convenience function for resolving blocks needed to cover a sample range.
 * @param range The sample range.
 * @param blockSize The size of the blocks constant for all blocks.
 */
pair<int64_t, int64_t> sampleRangeToBlockRange(pair<int64_t, int64_t> range, unsigned int blockSize)
{
	int64_t from = range.first;
	int64_t	to = range.second;

	from /= blockSize - 1;
	to = (to - 1)/(blockSize - 1);

	return make_pair(from, to);
}

void getEventsForRendering(OpenDataFile* file, int firstSample, int lastSample, vector<tuple<int, int, int>>* allChannelEvents, vector<tuple<int, int, int, int>>* singleChannelEvents)
{
	const AbstractEventTable* eventTable = getEventTable(file);

	for (int i = 0; i < eventTable->rowCount(); ++i)
	{
		Event e = eventTable->row(i);

		if (e.position <= lastSample && firstSample <= e.position + e.duration - 1 && e.type >= 0 && e.channel >= -1 &&
			file->dataModel->eventTypeTable()->row(e.type).hidden == false)
		{
			if (e.channel == - 1)
			{
				allChannelEvents->emplace_back(e.type, e.position, e.duration);
			}
			else
			{
				if (getTrackTable(file)->row(e.channel).hidden == false)
					singleChannelEvents->emplace_back(e.type, e.channel, e.position, e.duration);
			}
		}
	}

	sort(allChannelEvents->begin(), allChannelEvents->end(), [] (tuple<int, int, int> a, tuple<int, int, int> b) { return get<0>(a) < get<0>(b); });

	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<1>(a) < get<1>(b); });
	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<0>(a) < get<0>(b); });
}

} // namespace

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

void Canvas::changeFile(OpenDataFile* file)
{
	assert(signalProcessor);

	makeCurrent();

	this->file = file;
	signalProcessor->changeFile(file);

	if (file)
	{
		auto c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(updateFilter()));
		openFileConnections.push_back(c);
		c = connect(&OpenDataFile::infoTable, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(updateFilter()));
		openFileConnections.push_back(c);
		c = connect(&OpenDataFile::infoTable, SIGNAL(notchChanged(bool)), this, SLOT(updateFilter()));
		openFileConnections.push_back(c);
		c = connect(&OpenDataFile::infoTable, SIGNAL(filterWindowChanged(AlenkaSignal::WindowFunction)), this, SLOT(updateFilter()));
		openFileConnections.push_back(c);

		c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(selectMontage()));
		openFileConnections.push_back(c);

		c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this, SLOT(updateCursor()));
		openFileConnections.push_back(c);

		samplesRecorded = file->file->getSamplesRecorded();
		samplingFrequency = file->file->getSamplingFrequency();
	}
	else
	{
		for (auto e : openFileConnections)
			disconnect(e);
		openFileConnections.clear();
	}

	doneCurrent();
}

QColor Canvas::modifySelectionColor(const QColor& color)
{
	double colorComponents[3] = {color.redF(), color.greenF(), color.blueF()};

	for (int i = 0; i < 3; ++i)
		colorComponents[i] += colorComponents[i] > 0.5 ? -0.45 : 0.45;

	QColor newColor(color);
	newColor.setRedF(colorComponents[0]);
	newColor.setGreenF(colorComponents[1]);
	newColor.setBlueF(colorComponents[2]);
	return newColor;
}

void Canvas::horizontalZoom(bool reverse)
{
	double factor = horizontalZoomFactor;
	if (reverse)
		factor = 1/factor;

	OpenDataFile::infoTable.setVirtualWidth(OpenDataFile::infoTable.getVirtualWidth()*factor);
}

void Canvas::verticalZoom(bool reverse)
{
	if (file)
	{
		double factor = verticalZoomFactor;
		if (reverse)
			factor = 1/factor;

		for (int i = 0; i < getTrackTable(file)->rowCount(); ++i)
			zoom(file, factor, i);
	}
}

void Canvas::trackZoom(bool reverse)
{
	if (file)
	{
		double factor = trackZoomFactor;
		if (reverse)
			factor = 1/factor;

		int track = cursorTrack;
		track += countHiddenTracks(track);

		zoom(file, factor, track);
	}
}

void Canvas::updateCursor()
{
	if (ready())
	{
		QPoint pos = mapFromGlobal(QCursor::pos());

		double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();
		int sample = round((pos.x() + OpenDataFile::infoTable.getPosition())*ratio);

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

	QFile signalVertFile(":/signal.vert");
	signalVertFile.open(QIODevice::ReadOnly);
	string signalVert = signalVertFile.readAll().toStdString();

	QFile eventVertFile(":/event.vert");
	eventVertFile.open(QIODevice::ReadOnly);
	string eventVert = eventVertFile.readAll().toStdString();

	QFile rectangleLineVertFile(":/rectangleLine.vert");
	rectangleLineVertFile.open(QIODevice::ReadOnly);
	string rectangleLineVert = rectangleLineVertFile.readAll().toStdString();

	QFile colorFragFile(":/color.frag");
	colorFragFile.open(QIODevice::ReadOnly);
	string colorFrag = colorFragFile.readAll().toStdString();

	signalProgram = new OpenGLProgram(signalVert, colorFrag);
	eventProgram = new OpenGLProgram(eventVert, colorFrag);
	rectangleLineProgram = new OpenGLProgram(rectangleLineVert, colorFrag);

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

#if defined GL_2_0
	const GLubyte* str = gl()->glGetString(GL_EXTENSIONS);
	if (str)
		ss << "Extensions: " << str << endl;
	else
		ss << "Extensions:" << endl;
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
		double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();

		QMatrix4x4 matrix;
		matrix.ortho(QRectF(OpenDataFile::infoTable.getPosition()*ratio, 0, width()*ratio, height()));

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
		int firstSample = static_cast<unsigned int>(floor(OpenDataFile::infoTable.getPosition()*ratio));
		int lastSample = static_cast<unsigned int>(ceil((OpenDataFile::infoTable.getPosition() + width())*ratio));

		auto fromTo = sampleRangeToBlockRange(make_pair(firstSample, lastSample), signalProcessor->getBlockSize());

		set<int> indexSet;

		for (int i = fromTo.first; i <= fromTo.second; ++i)
			indexSet.insert(i);

		// Get events.
		vector<tuple<int, int, int>> allChannelEvents;
		vector<tuple<int, int, int, int>> singleChannelEvents;
		getEventsForRendering(file, firstSample, lastSample, &allChannelEvents, &singleChannelEvents);

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
	bool isDown;

	if (event->angleDelta().isNull() == false)
	{
		isDown = event->angleDelta().x() + event->angleDelta().y() < 0;
	}
	else if (event->pixelDelta().isNull() == false)
	{
		isDown = event->pixelDelta().x() + event->pixelDelta().y() < 0;
	}
	else
	{
		event->ignore();
		return;
	}

	if (event->modifiers() & Qt::ControlModifier)
	{
		trackZoom(isDown);

		update();
		event->accept();
		return;
	}
	else if (event->modifiers() & Qt::AltModifier)
	{
		horizontalZoom(isDown);

		updateCursor();
		emit OpenDataFile::infoTable.positionIndicatorChanged(OpenDataFile::infoTable.getPositionIndicator());

		update();
		event->accept();
		return;
	}
	else if (event->modifiers() & Qt::ShiftModifier)
	{
		verticalZoom(isDown);

		update();
		event->accept();
		return;
	}

	event->ignore();
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
		isDrawingCross = !isDrawingCross; // TODO: Perhaps promote this to an action?
		update();
	}
	else if (ready() && event->key() == Qt::Key_T)
	{
		QPoint pos = mapFromGlobal(QCursor::pos());
		double indicator = static_cast<double>(pos.x())/width();

		if (0 <= indicator && indicator <= 1)
		{
			OpenDataFile::infoTable.setPositionIndicator(indicator);
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
			update();
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

				double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();
				eventStart = eventEnd = (event->pos().x() + OpenDataFile::infoTable.getPosition())*ratio;

				if (event->modifiers() == Qt::ShiftModifier)
					eventTrack = -1;
				else if (event->modifiers() == Qt::ControlModifier)
					eventTrack = cursorTrack;

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

void Canvas::drawAllChannelEvents(const vector<tuple<int, int, int>>& eventVector)
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
			QColor color; double opacity;
			getEventTypeColorOpacity(file, type, &color, &opacity);
			setUniformColor(rectangleLineProgram->getGLProgram(), color, opacity);
		}
	}

	if (isDrawingEvent)
	{
		if (eventTrack == -1)
		{
			QColor color(Qt::blue);
			double opacity = 0.5;
			int type = OpenDataFile::infoTable.getSelectedType();
			if (type != -1)
			{
				getEventTypeColorOpacity(file, type, &color, &opacity);
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
	double interval = OpenDataFile::infoTable.getTimeLineInterval();

	gl()->glUseProgram(rectangleLineProgram->getGLProgram());
	glBindVertexArray(rectangleLineArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, rectangleLineBuffer);

	setUniformColor(rectangleLineProgram->getGLProgram(), QColor(Qt::green), 1);

	double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();
	interval *= samplingFrequency;

	double position = OpenDataFile::infoTable.getPosition()*ratio;
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

	double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();
	double position = (OpenDataFile::infoTable.getPosition() + width()*OpenDataFile::infoTable.getPositionIndicator())*ratio;

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

	double ratio = samplesRecorded/OpenDataFile::infoTable.getVirtualWidth();

	double position = (OpenDataFile::infoTable.getPosition() + pos.x())*ratio;

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

	const AbstractTrackTable* trackTable = getTrackTable(file);
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
				assert(trackTable->row(track).hidden == false);

				hidden = 0;
				for (int i = 0; i < track; ++i)
				{
					if (trackTable->row(i).hidden)
						++hidden;
				}

				setUniformTrack(eventProgram->getGLProgram(), track, hidden, block);
			}
		}
		else
		{
			type = get<0>(eventVector[event]);
			QColor color; double opacity;
			getEventTypeColorOpacity(file, type, &color, &opacity);
			setUniformColor(eventProgram->getGLProgram(), color, opacity);
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
			int type = OpenDataFile::infoTable.getSelectedType();
			if (type != -1)
				getEventTypeColorOpacity(file, type, &color, &opacity);
			setUniformColor(eventProgram->getGLProgram(), color, opacity);

			int start = eventStart, end = eventEnd;

			if (end < start)
				swap(start, end);

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

		QColor color = DataModel::array2color<QColor>(getTrackTable(file)->row(track + hidden).color);
		if (isSelectingTrack && track == cursorTrack)
			color = modifySelectionColor(color);

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
	float yScale = getTrackTable(file)->row(track).amplitude;
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

void Canvas::checkGLMessages()
{
	for (const auto& m : log()->loggedMessages())
		logToFile("OpenGL message: " << m.message().toStdString());
}

void Canvas::addEvent(int channel)
{
	file->undoFactory->beginMacro("add event");

	const AbstractEventTable* eventTable = getEventTable(file);

	int index = eventTable->rowCount();
	file->undoFactory->insertEvent(OpenDataFile::infoTable.getSelectedMontage(), index);

	Event e = eventTable->row(index);

	e.type = OpenDataFile::infoTable.getSelectedType();
	if (eventEnd < eventStart)
		swap(eventStart, eventEnd);
	e.position = eventStart;
	e.duration = eventEnd - eventStart + 1;
	e.channel = channel + countHiddenTracks(channel);

	file->undoFactory->changeEvent(OpenDataFile::infoTable.getSelectedMontage(), index, e);

	file->undoFactory->endMacro();
}

int Canvas::countHiddenTracks(int track)
{
	int hidden = 0;
	int i = 0;

	while (i - hidden <= track)
	{
		if (getTrackTable(file)->row(i++).hidden)
			++hidden;
	}

	return hidden;
}

void Canvas::updateFilter()
{
	assert(signalProcessor);

	makeCurrent();
	signalProcessor->updateFilter();
	doneCurrent();
}

void Canvas::selectMontage()
{
	for (auto e : montageConnections)
		disconnect(e);
	montageConnections.clear();

	if (file && 0 < file->dataModel->montageTable()->rowCount())
	{
		auto vitness = VitnessTrackTable::vitness(getTrackTable(file));

		auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(updateMontage(int, int)));
		montageConnections.push_back(c);
		c = connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(updateMontage()));
		montageConnections.push_back(c);
		c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this, SLOT(updateMontage()));
		montageConnections.push_back(c);
	}

	updateMontage();
}

void Canvas::updateMontage(int row, int col)
{
	(void)row;
	Track::Index column = static_cast<Track::Index>(col);
	if (column == Track::Index::code || column == Track::Index::hidden)
		updateMontage();
}

void Canvas::updateMontage()
{
	assert(signalProcessor);

	makeCurrent();
	signalProcessor->setUpdateMontageFlag();
	doneCurrent();
}

bool Canvas::ready()
{
	return /*file && 0 < file->dataModel->montageTable()->rowCount() &&*/ signalProcessor && signalProcessor->ready();
}
