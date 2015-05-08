#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include "../SignalProcessor/filter.h"

class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief A class for handling program wide used information stored in .info files.
 */
class InfoTable : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Enum defining symbolic constants for modes of time output.
	 */
	enum class TimeMode
	{
		samples, offset, real, size
	};

	InfoTable();
	~InfoTable();

	/**
	 * @brief Writes an infoTable element.
	 */
	void write(QXmlStreamWriter* xml) const;

	/**
	 * @brief Parses the infoTable element.
	 */
	void read(QXmlStreamReader* xml);

	/**
	 * @brief Emit all signals defined by this class.
	 *
	 * This method can be used to ensure that all components referencing values
	 * governed by InfoTable are synchronized.
	 */
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
		emit timeLineIntervalChanged(timeLineInterval);
		emit positionIndicatorChanged(positionIndicator);
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
	double getTimeLineInterval() const
	{
		return timeLineInterval;
	}
	double getPositionIndicator() const
	{
		return positionIndicator;
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
	void timeLineIntervalChanged(double);
	void positionIndicatorChanged(double);

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
	void setTimeLineInterval(double value)
	{
		if (value != timeLineInterval)
		{
			timeLineInterval = value;
			emit timeLineIntervalChanged(value);
		}
	}
	void setPositionIndicator(double value)
	{
		if (value != positionIndicator)
		{
			positionIndicator = value;
			emit positionIndicatorChanged(value);
		}
	}

private:
	int virtualWidth = 100000;
	int position = 0;
	double lowpassFrequency = Filter::LOWPASS_OFF_VALUE;
	double highPassFrequency = Filter::HIGHPASS_OFF_VALUE;
	bool notch = false;
	int selectedMontage = 0;
	TimeMode timeMode = TimeMode::offset;
	int selectedType = 0;
	double timeLineInterval = 1;
	double positionIndicator = 0.5;
};

#endif // INFOTABLE_H
