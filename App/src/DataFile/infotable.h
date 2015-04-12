#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

class InfoTable : public QObject
{
	Q_OBJECT

public:
	enum class TimeMode
	{
		samples, offset, real, size
	};

	InfoTable();
	~InfoTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	void emitAllSignals()
	{
		emit virtualWidthChanged(virtualWidth);
		emit positionChanged(position);
		emit lowpassFrequencyChanged(lowpassFrequency);
		emit highpassFrequencyChanged(highPassFrequency);
		emit notchChanged(notch);
		emit selectedMontageChanged(selectedMontage);
		emit timeModeChanged(static_cast<int>(timeMode));
		emit selectedTypeChanged(selectedType);
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
	double getHighpassFrequency() const
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
	TimeMode getTimeMode() const
	{
		return timeMode;
	}
	int getSelectedType() const
	{
		return selectedType;
	}

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);
	void lowpassFrequencyChanged(double);
	void highpassFrequencyChanged(double);
	void notchChanged(bool);
	void selectedMontageChanged(int);
	void timeModeChanged(int);
	void selectedTypeChanged(int);

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
	void setHighpassFrequency(double value)
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
	void setTimeMode(TimeMode value)
	{
		if (value != timeMode)
		{
			timeMode = value;
			emit timeModeChanged(static_cast<int>(value));
		}
	}
	void setSelectedType(int value)
	{
		if (value != selectedType)
		{
			selectedType = value;
			emit selectedTypeChanged(value);
		}
	}

private:
	int virtualWidth = 100000;
	int position = 0;
	double lowpassFrequency = 1000*1000*1000; // TODO: add on/off vars to avoid using these arbitrary values
	double highPassFrequency = -1000*1000*1000;
	bool notch = false;
	int selectedMontage = 0;
	TimeMode timeMode = TimeMode::offset;
	int selectedType = 0;
};

#endif // INFOTABLE_H
