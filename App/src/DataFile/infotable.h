#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class InfoTable : public QObject
{
	Q_OBJECT

public:
	InfoTable();
	~InfoTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);

	int getVirtualWidth() const
	{
		return virtualWidth;
	}
	int getPosition() const
	{
		return position;
	}
	double getLowpassFrequency() const
	{
		return lowpassFrequency;
	}
	double getHighFrequency() const
	{
		return highPassFrequency;
	}
	bool getNotch() const
	{
		return notch;
	}

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);
	void lowpassFrequencyChanged(double);
	void highpassFrequencyChanged(double);
	void notchChanged(bool);

public slots:
	void setVirtualWidth(int value)
	{
		if (value != virtualWidth)
		{
			virtualWidth = value;
			emit virtualWidthChanged(virtualWidth);
		}
	}
	void setPosition(int value)
	{
		if (value != position)
		{
			position = value;
			emit positionChanged(position);
		}
	}
	void setLowpassFrequency(double value)
	{
		if (value != lowpassFrequency)
		{
			lowpassFrequency = value;
			emit lowpassFrequencyChanged(lowpassFrequency);
		}
	}
	void setHighFrequency(double value)
	{
		if (value != highPassFrequency)
		{
			highPassFrequency = value;
			emit highpassFrequencyChanged(highPassFrequency);
		}
	}
	void setNotch(bool value)
	{
		if (value != notch)
		{
			notch = value;
			emit notchChanged(notch);
		}
	}

private:
	int virtualWidth = 100000;
	int position = 0;
	double lowpassFrequency = 1000*1000*1000; // TODO: add on/off vars to avoid using these arbitrary values
	double highPassFrequency = -1000*1000*1000;
	bool notch = false;
};

#endif // INFOTABLE_H
