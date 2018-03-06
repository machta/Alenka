#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include "../../Alenka-Signal/include/AlenkaSignal/filterprocessor.h"

#include <string>
#include <utility>
#include <vector>

using DETECTOR_SETTINGS = struct detectorSettings;
using dpair = std::pair<double, double>;

/**
 * @brief A class for handling program-wide (i.e. global) information.
 *
 * Most of this info is stored in .info files.
 */
class InfoTable : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Enum defining symbolic constants for modes of time output.
   */
  enum class TimeMode { samples, offset, real, size };

private:
  int virtualWidth;
  int position; // TODO: Start using int64_t for tracking the position.
  double positionIndicator; // TODO: Change all these doubles to floats.
  double lowpassFrequency;
  bool lowpassOn;
  double highpassFrequency;
  bool highpassOn;
  bool notchOn; // TODO: Add notch frequency.
  AlenkaSignal::WindowFunction filterWindow;
  int selectedMontage;
  InfoTable::TimeMode timeMode;
  int selectedType;
  double timeLineInterval;
  std::vector<dpair> frequencyMultipliers;
  bool frequencyMultipliersOn;
  float sampleScale;
  int sampleUnits;
  QString elkoSession;

  // The following values are not saved to .info files.
  int pixelViewWidth;
  std::vector<float> filterCoefficients;
  QString globalMontageHeader;

public:
  InfoTable(QObject *parent = nullptr) : QObject(parent) { setDefaultValues(); }

  void setDefaultValues();
  /**
   * @brief Emit all signals defined by this class.
   *
   * This method can be used to ensure that all components referencing values
   * governed by InfoTable are synchronized.
   */
  void emitAllSignals();

  /**
   * @brief Writes info file.
   */
  void writeXML(const std::string &filePath,
                const DETECTOR_SETTINGS &spikedetSettings,
                double spikeDuration) const;
  /**
   * @brief Reads info file.
   */
  void readXML(const std::string &filePath, DETECTOR_SETTINGS *spikedetSettings,
               double *spikeDuration);

  int getVirtualWidth() const { return virtualWidth; }
  int getPosition() const { return position; }
  double getPositionIndicator() const { return positionIndicator; }
  double getLowpassFrequency() const { return lowpassFrequency; }
  bool getLowpassOn() const { return lowpassOn; }
  double getHighpassFrequency() const { return highpassFrequency; }
  bool getHighpassOn() const { return highpassOn; }
  bool getNotchOn() const { return notchOn; }
  AlenkaSignal::WindowFunction getFilterWindow() const { return filterWindow; }
  int getSelectedMontage() const { return selectedMontage; }
  InfoTable::TimeMode getTimeMode() const { return timeMode; }
  int getSelectedType() const { return selectedType; }
  double getTimeLineInterval() const { return timeLineInterval; }
  const std::vector<dpair> &getFrequencyMultipliers() const {
    return frequencyMultipliers;
  }
  bool getFrequencyMultipliersOn() const { return frequencyMultipliersOn; }
  float getSampleScale() const { return sampleScale; }
  int getSampleUnits() const { return sampleUnits; }
  QString getElkoSession() const { return elkoSession; }

  int getPixelViewWidth() const { return pixelViewWidth; }
  const std::vector<float> &getFilterCoefficients() const {
    return filterCoefficients;
  }
  QString getGlobalMontageHeader() const { return globalMontageHeader; }

signals:
  void virtualWidthChanged(int);
  //! Gets triggered when position and/or positionIndicator change
  void positionChanged(int, double);
  void lowpassFrequencyChanged(double);
  void lowpassOnChanged(bool);
  void highpassFrequencyChanged(double);
  void highpassOnChanged(bool);
  void notchOnChanged(bool);
  void filterWindowChanged(AlenkaSignal::WindowFunction);
  void selectedMontageChanged(int);
  void timeModeChanged(InfoTable::TimeMode);
  void selectedTypeChanged(int);
  void timeLineIntervalChanged(double);
  void frequencyMultipliersChanged();
  void frequencyMultipliersOnChanged(bool);
  void sampleScaleChanged(float);
  void sampleUnitsChanged(int);
  void elkoSessionChanged(QString);

  void pixelViewWidthChanged(int);
  void filterCoefficientsChanged();
  void globalMontageHeaderChanged(QString);

public slots:
  // TODO: Perhaps use macros to lessen the code duplicity for the properties.
  void setVirtualWidth(int value) {
    if (value != virtualWidth) {
      virtualWidth = value;
      emit virtualWidthChanged(value);
    }
  }
  /**
   * @brief Position and indicator are coupled (i.e. changing one effects the
   * other) so they must be set at the same time. They also share the signal.
   */
  void setPosition(int position, double positionIndicator = -1) {
    if (positionIndicator < 0) // This allows omitting the second parameter.
      positionIndicator = this->positionIndicator;

    if (position != this->position ||
        positionIndicator != this->positionIndicator) {
      this->position = position;
      this->positionIndicator = positionIndicator;
      emit positionChanged(position, positionIndicator);
    }
  }
  void setLowpassFrequency(double value) {
    if (value != lowpassFrequency) {
      lowpassFrequency = value;
      emit lowpassFrequencyChanged(value);
    }
  }
  void setLowpassOn(bool value) {
    if (value != lowpassOn) {
      lowpassOn = value;
      emit lowpassOnChanged(value);
    }
  }
  void setHighpassFrequency(double value) {
    if (value != highpassFrequency) {
      highpassFrequency = value;
      emit highpassFrequencyChanged(value);
    }
  }
  void setHighpassOn(bool value) {
    if (value != highpassOn) {
      highpassOn = value;
      emit highpassOnChanged(value);
    }
  }
  void setNotchOn(bool value) {
    if (value != notchOn) {
      notchOn = value;
      emit notchOnChanged(value);
    }
  }
  void setFilterWindow(AlenkaSignal::WindowFunction value) {
    if (value != filterWindow) {
      filterWindow = value;
      emit filterWindowChanged(value);
    }
  }
  void setSelectedMontage(int value) {
    if (value != selectedMontage) {
      selectedMontage = value;
      emit selectedMontageChanged(value);
    }
  }
  void setTimeMode(InfoTable::TimeMode value) {
    if (value != timeMode) {
      timeMode = value;
      emit timeModeChanged(value);
    }
  }
  void setSelectedType(int value) {
    if (value != selectedType) {
      selectedType = value;
      emit selectedTypeChanged(value);
    }
  }
  void setTimeLineInterval(double value) {
    if (value != timeLineInterval) {
      timeLineInterval = value;
      emit timeLineIntervalChanged(value);
    }
  }
  void setFrequencyMultipliers(const std::vector<dpair> &value) {
    if (value != frequencyMultipliers) {
      frequencyMultipliers = value;
      emit frequencyMultipliersChanged();
    }
  }
  void setFrequencyMultipliersOn(bool value) {
    if (value != frequencyMultipliersOn) {
      frequencyMultipliersOn = value;
      emit frequencyMultipliersOnChanged(value);
    }
  }
  void setSampleScale(float value) {
    if (value != sampleScale) {
      sampleScale = value;
      emit sampleScaleChanged(value);
    }
  }
  void setSampleUnits(int value) {
    if (value != sampleUnits) {
      sampleUnits = value;
      emit sampleUnitsChanged(value);
    }
  }
  void setElkoSession(const QString &value) {
    if (value != elkoSession) {
      elkoSession = value;
      emit elkoSessionChanged(value);
    }
  }

  void setPixelViewWidth(int value) {
    if (value != pixelViewWidth) {
      pixelViewWidth = value;
      emit pixelViewWidthChanged(value);
    }
  }
  void setFilterCoefficients(const std::vector<float> &value) {
    if (value != filterCoefficients) {
      filterCoefficients = value;
      emit filterCoefficientsChanged();
    }
  }
  void setGlobalMontageHeader(const QString &value) {
    if (value != globalMontageHeader) {
      globalMontageHeader = value;
      emit globalMontageHeaderChanged(value);
    }
  }
};

#endif // INFOTABLE_H
