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

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);

public slots:
	void changeFile(DataFile* file);

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
	void resize(int virtualWidth)
	{
		int page = canvas->size().width();

		scrollBar->setRange(0, virtualWidth - page - 1);
		scrollBar->setPageStep(page);
		scrollBar->setSingleStep(std::max(1, page/20));
	}

private slots:
	void setVirtualWidth(int value)
	{
		double relPosition = scrollBar->value();
		relPosition /= (scrollBar->maximum() - scrollBar->minimum() + 1);

		resize(value);

		setPosition(static_cast<int>(std::round(relPosition*(scrollBar->maximum() - scrollBar->minimum() + 1))));

		canvas->update();
	}
	void setPosition(int value)
	{
		scrollBar->setValue(value);

		canvas->update();
	}
};

#endif // SIGNALVIEWER_H
