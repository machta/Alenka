#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

class InfoTable : public QObject
{
	Q_OBJECT

public:
	InfoTable();
	~InfoTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	void emitAllSignals()
	{
		emit virtualWidthChanged(getVirtualWidth());
		emit positionChanged(getPosition());
		emit lowpassFrequencyChanged(getLowpassFrequency());
		emit highpassFrequencyChanged(getHighFrequency());
		emit notchChanged(getNotch());
		emit selectedMontageChanged(getSelectedMontage());
	}

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
	int getSelectedMontage() const
	{
		return selectedMontage;
	}

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);
	void lowpassFrequencyChanged(double);
	void highpassFrequencyChanged(double);
	void notchChanged(bool);
	void selectedMontageChanged(int);

public slots:
	void setVirtualWidth(int value)
	{
		if (value != virtualWidth)
		{
			virtualWidth = value;
			emit virtualWidthChanged(value);
		}
	}
	void setPosition(int value)
	{
		if (value != position)
		{
			position = value;
			emit positionChanged(value);
		}
	}
	void setLowpassFrequency(double value)
	{
		if (value != lowpassFrequency)
		{
			lowpassFrequency = value;
			emit lowpassFrequencyChanged(value);
		}
	}
	void setHighFrequency(double value)
	{
		if (value != highPassFrequency)
		{
			highPassFrequency = value;
			emit highpassFrequencyChanged(value);
		}
	}
	void setNotch(bool value)
	{
		if (value != notch)
		{
			notch = value;
			emit notchChanged(value);
		}
	}
	void setSelectedMontage(int value)
	{
		if (value != selectedMontage)
		{
			selectedMontage = value;
			emit selectedMontageChanged(value);
		}
	}

private:
	int virtualWidth = 100000;
	int position = 0;
	double lowpassFrequency = 1000*1000*1000; // TODO: add on/off vars to avoid using these arbitrary values
	double highPassFrequency = -1000*1000*1000;
	bool notch = false;
	int selectedMontage = 0;
};

#endif // INFOTABLE_H
