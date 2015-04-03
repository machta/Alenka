#ifndef SIGNALVIEWER_H
#define SIGNALVIEWER_H

#include <QWidget>
#include <QScrollBar>
#include <QVBoxLayout>

#include "canvas.h"
#include "SignalProcessor/signalprocessor.h"
#include "DataFile/datafile.h"
#include "DataFile/infotable.h"

#include <algorithm>
#include <cmath>

class SignalViewer : public QWidget
{
	Q_OBJECT

public:
	explicit SignalViewer(QWidget* parent = 0);
	~SignalViewer();

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);

public slots:
	void changeFile(DataFile* file)
	{
		canvas->changeFile(file);

		if (file != nullptr)
		{
			infoTable = file->getInfoTable();

			connect(scrollBar, SIGNAL(valueChanged(int)), infoTable, SLOT(setPosition(int)));

			connect(infoTable, SIGNAL(virtualWidthChanged(int)), this, SLOT(setVirtualWidth(int)));
			connect(infoTable, SIGNAL(positionChanged(int)), this, SLOT(setPosition(int)));
		}
		else
		{
			infoTable = nullptr;
		}
	}

protected:
	virtual void resizeEvent(QResizeEvent* /*event*/) override
	{
		resize(getInfoTable()->getVirtualWidth());
	}

private:
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	Canvas* canvas;
	QScrollBar* scrollBar;

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
