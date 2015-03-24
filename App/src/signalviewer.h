#ifndef SIGNALVIEWER_H
#define SIGNALVIEWER_H

#include <QWidget>
#include <QScrollBar>
#include <QVBoxLayout>

#include "canvas.h"
#include "SignalProcessor/signalprocessor.h"
#include "DataFile/datafile.h"

#include <algorithm>

class SignalViewer : public QWidget
{
	Q_OBJECT

public:
	explicit SignalViewer(QWidget* parent = 0);
	~SignalViewer();

	int getVirtualWidth() const
	{
		return virtualWidth;
	}
	int getPosition() const
	{
		return position;
	}

protected:
	virtual void resizeEvent(QResizeEvent*)
	{
		updateSignalViewer();
	}

private:
	Canvas* canvas;
	QScrollBar* scrollBar;
	QVBoxLayout* box;
	int virtualWidth = 10000;
	int position = 0;

	void updateSignalViewer()
	{
		int page = canvas->size().width();

		scrollBar->setRange(0, virtualWidth - page);
		scrollBar->setPageStep(page);
		scrollBar->setSingleStep(std::max(1, page/20));
		//scrollBar->setValue();

		canvas->update();
	}

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);

public slots:
	void setVirtualWidth(int value)
	{
		if (value != virtualWidth)
		{
			virtualWidth = value;
			updateSignalViewer();
			emit virtualWidthChanged(virtualWidth);
		}
	}
	void setPosition(int value)
	{
		if (value != position)
		{
			position = value;
			updateSignalViewer();
			emit positionChanged(position);
		}
	}
	void changeFile(DataFile* file)
	{
		canvas->changeFile(file);
	}
};

#endif // SIGNALVIEWER_H
