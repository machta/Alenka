#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include <AlenkaSignal/filterprocessor.h>

#include <string>

namespace AlenkaSignal
{
typedef struct detectorSettings DETECTOR_SETTINGS;
}

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

	/**
	 * @brief Writes info file.
	 */
	void writeXML(const std::string& filePath, const AlenkaSignal::DETECTOR_SETTINGS& spikedetSettings, double spikeDuration) const;

	/**
	 * @brief Reads info file.
	 */
	void readXML(const std::string& filePath, AlenkaSignal::DETECTOR_SETTINGS* spikedetSettings, double* spikeDuration);

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
		emit filterWindowChanged(filterWindow);
		emit selectedMontageChanged(selectedMontage);
		emit timeModeChanged(timeMode);
		emit selectedTypeChanged(selectedType);
		emit timeLineIntervalChanged(timeLineInterval);
		emit positionIndicatorChanged(positionIndicator);
		emit pixelViewWidthChanged(pixelViewWidth);
		emit frequencyMultipliersChanged();
		emit frequencyMultipliersOnChanged(frequencyMultipliersOn);
	}

	int getVirtualWidth() const { return virtualWidth; }
	int getPosition() const { return position; }
	double getLowpassFrequency() const { return lowpassFrequency; }
	double getHighpassFrequency() const { return highPassFrequency; }
	bool getNotch() const {	return notch; }
	AlenkaSignal::WindowFunction getFilterWindow() const { return filterWindow; }
	int getSelectedMontage() const { return selectedMontage; }
	InfoTable::TimeMode getTimeMode() const { return timeMode;	}
	int getSelectedType() const { return selectedType; }
	double getTimeLineInterval() const { return timeLineInterval; }
	double getPositionIndicator() const { return positionIndicator; }
	int getPixelViewWidth() const { return pixelViewWidth; }
	const std::vector<std::pair<double, double>>& getFrequencyMultipliers() const { return frequencyMultipliers; }
	bool getFrequencyMultipliersOn() const { return frequencyMultipliersOn; }
	const std::vector<float>& getFilterCoefficients() const { return filterCoefficients; }

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);
	void lowpassFrequencyChanged(double);
	void highpassFrequencyChanged(double);
	void notchChanged(bool);
	void filterWindowChanged(AlenkaSignal::WindowFunction);
	void selectedMontageChanged(int);
	void timeModeChanged(InfoTable::TimeMode);
	void selectedTypeChanged(int);
	void timeLineIntervalChanged(double);
	void positionIndicatorChanged(double);
	void pixelViewWidthChanged(int);
	void frequencyMultipliersChanged();
	void frequencyMultipliersOnChanged(bool);
	void filterCoefficientsChanged();

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
	void setFilterWindow(AlenkaSignal::WindowFunction value)
	{
		if (value != filterWindow)
		{
			filterWindow = value;
			emit filterWindowChanged(value);
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
	void setTimeMode(InfoTable::TimeMode value)
	{
		if (value != timeMode)
		{
			timeMode = value;
			emit timeModeChanged(value);
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
	void setPixelViewWidth(int value)
	{
		if (value != pixelViewWidth)
		{
			pixelViewWidth = value;
			emit pixelViewWidthChanged(value);
		}
	}
	void setFrequencyMultipliers(const std::vector<std::pair<double, double>>& value)
	{
		if (value != frequencyMultipliers)
		{
			frequencyMultipliers = value;
			emit frequencyMultipliersChanged();
		}
	}
	void setFrequencyMultipliersOn(bool value)
	{
		if (value != frequencyMultipliersOn)
		{
			frequencyMultipliersOn = value;
			emit frequencyMultipliersOnChanged(value);
		}
	}
	void setFilterCoefficients(const std::vector<float>& value)
	{
		filterCoefficients = value;
		emit filterCoefficientsChanged();
	}

private:
	int virtualWidth = 100000;
	int position = 0;
	double lowpassFrequency = 1000000;
	double highPassFrequency = -1000000;
	bool notch = false;
	AlenkaSignal::WindowFunction filterWindow = AlenkaSignal::WindowFunction::None;
	int selectedMontage = 0;
	InfoTable::TimeMode timeMode = InfoTable::TimeMode::offset;
	int selectedType = 0;
	double timeLineInterval = 1;
	double positionIndicator = 0.5;
	std::vector<std::pair<double, double>> frequencyMultipliers;
	bool frequencyMultipliersOn = true;

	// The following values are not saved to .info files.
	int pixelViewWidth = 0;
	std::vector<float> filterCoefficients;
};

#endif // INFOTABLE_H
