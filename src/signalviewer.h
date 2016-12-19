#ifndef SIGNALVIEWER_H
#define SIGNALVIEWER_H

#include <QWidget>

#include "canvas.h"
#include "SignalProcessor/signalprocessor.h"
#include "DataFile/infotable.h"

#include <QScrollBar>

#include <algorithm>
#include <cmath>

class DataFile;
class TrackLabelBar;
class QScrollBar;
class QSplitter;

/**
 * @brief This class implements the GUI control for browsing the DataFile's signal.
 *
 * This control combines in itself a Canvas, a QScrollBar, and a TrackLabelBar.
 * It constructs them, connects the appropriate signals and slots and
 * dispatches events between Canvas and the QScrollBar.
 *
 * Also resizing and handling of virtual width and position is done here.
 */
class SignalViewer : public QWidget
{
	Q_OBJECT

public:
	explicit SignalViewer(QWidget* parent = nullptr);
	~SignalViewer();

	const Canvas* getCanvas() const
	{
		return canvas;
	}

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(DataFile* file);

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);

public slots:	

protected:
	virtual void resizeEvent(QResizeEvent* /*event*/) override
	{
		resize(getInfoTable()->getVirtualWidth());
	}
	virtual void wheelEvent(QWheelEvent* event) override
	{
		scrollBar->event(reinterpret_cast<QEvent*>(event));
	}
	virtual void keyPressEvent(QKeyEvent* event) override
	{
		scrollBar->event(reinterpret_cast<QEvent*>(event));
	}
	virtual void keyReleaseEvent(QKeyEvent* event) override
	{
		scrollBar->event(reinterpret_cast<QEvent*>(event));
	}

private:
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	QSplitter* splitter;
	Canvas* canvas;
	QScrollBar* scrollBar;
	TrackLabelBar* trackLabelBar;

	InfoTable* getInfoTable()
	{
		if (infoTable != nullptr)
		{
			return infoTable;
		}
		else
		{
			return &defaultInfoTable;
		}
	}
	void resize(int virtualWidth);
	int virtualWidthFromScrollBar()
	{
		return scrollBar->maximum() + scrollBar->pageStep() + 1 - scrollBar->minimum();
	}

private slots:
	void setVirtualWidth(int value);
	void setPosition(int value)
	{
		scrollBar->setValue(value);

		canvas->update();
	}
};

#endif // SIGNALVIEWER_H
