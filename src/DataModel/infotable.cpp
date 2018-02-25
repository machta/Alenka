#include "infotable.h"

#include "../../Alenka-Signal/include/AlenkaSignal/spikedet.h"
#include "../error.h"

#include <pugixml.hpp>

using namespace std;
using namespace pugi;

void InfoTable::setDefaultValues() {
  virtualWidth = 100000;
  position = 0;
  positionIndicator = 0.5;
  lowpassFrequency = 1000000;
  lowpassOn = false;
  highpassFrequency = -1000000;
  highpassOn = false;
  notchOn = false;
  filterWindow = AlenkaSignal::WindowFunction::None;
  selectedMontage = 1;
  timeMode = InfoTable::TimeMode::offset;
  selectedType = 0;
  timeLineInterval = 1;
  frequencyMultipliers.clear();
  frequencyMultipliersOn = true;
  sampleScale = 1;
  sampleUnits = 1;
  elkoSession.clear();

  pixelViewWidth = 0;
  filterCoefficients.clear();
  globalMontageHeader.clear();
}

void InfoTable::emitAllSignals() {
  emit virtualWidthChanged(virtualWidth);
  emit positionChanged(position, positionIndicator);
  if (lowpassOn)
    emit lowpassFrequencyChanged(lowpassFrequency);
  emit lowpassOnChanged(lowpassOn);
  if (highpassOn)
    emit highpassFrequencyChanged(highpassFrequency);
  emit highpassOnChanged(highpassOn);
  emit notchOnChanged(notchOn);
  emit filterWindowChanged(filterWindow);
  emit selectedMontageChanged(selectedMontage);
  emit timeModeChanged(timeMode);
  emit selectedTypeChanged(selectedType);
  emit timeLineIntervalChanged(timeLineInterval);
  emit frequencyMultipliersChanged();
  emit frequencyMultipliersOnChanged(frequencyMultipliersOn);
  emit sampleScaleChanged(sampleScale);
  emit sampleUnitsChanged(sampleUnits);
  emit elkoSessionChanged(elkoSession);

  emit pixelViewWidthChanged(pixelViewWidth);
  emit frequencyMultipliersOnChanged(frequencyMultipliersOn);
  emit globalMontageHeaderChanged(globalMontageHeader);
}

void InfoTable::writeXML(const string &filePath,
                         const DETECTOR_SETTINGS &spikedetSettings,
                         double spikeDuration) const {
  xml_document file;
  xml_node document = file.append_child("document");

  xml_node browser = document.append_child("browser");
  browser.append_attribute("position").set_value(position);
  browser.append_attribute("positionIndicator").set_value(positionIndicator);
  browser.append_attribute("virtualWidth").set_value(virtualWidth);

  xml_node filter = document.append_child("filter");
  filter.append_attribute("filterWindow")
      .set_value(static_cast<int>(filterWindow));

  xml_node lowpass = filter.append_child("lowpass");
  lowpass.append_attribute("on").set_value(lowpassOn);
  lowpass.append_attribute("f").set_value(lowpassFrequency);

  xml_node highpass = filter.append_child("highpass");
  highpass.append_attribute("on").set_value(highpassOn);
  highpass.append_attribute("f").set_value(highpassFrequency);

  xml_node notch = filter.append_child("notch");
  notch.append_attribute("on").set_value(notchOn);
  // notch.append_attribute("f").set_value(notchFrequency);

  xml_node selection = document.append_child("selection");
  selection.append_attribute("selectedMontage").set_value(selectedMontage);
  selection.append_attribute("selectedType").set_value(selectedType);

  xml_node window = document.append_child("window");
  window.append_attribute("timeMode").set_value(static_cast<int>(timeMode));
  window.append_attribute("timeLineInterval").set_value(timeLineInterval);

  xml_node sample = document.append_child("sample");
  sample.append_attribute("scale").set_value(sampleScale);
  sample.append_attribute("units").set_value(sampleUnits);

  xml_node spikedet = document.append_child("spikedet");
  spikedet.append_attribute("fl").set_value(spikedetSettings.m_band_low);
  spikedet.append_attribute("fh").set_value(spikedetSettings.m_band_high);
  spikedet.append_attribute("k1").set_value(spikedetSettings.m_k1);
  spikedet.append_attribute("k2").set_value(spikedetSettings.m_k2);
  spikedet.append_attribute("k3").set_value(spikedetSettings.m_k3);
  spikedet.append_attribute("w").set_value(spikedetSettings.m_winsize);
  spikedet.append_attribute("n").set_value(spikedetSettings.m_noverlap);
  spikedet.append_attribute("buf").set_value(spikedetSettings.m_buffering);
  spikedet.append_attribute("h").set_value(spikedetSettings.m_main_hum_freq);
  spikedet.append_attribute("dt").set_value(spikedetSettings.m_discharge_tol);
  spikedet.append_attribute("pt").set_value(
      spikedetSettings.m_polyspike_union_time);
  spikedet.append_attribute("dec").set_value(spikedetSettings.m_decimation);
  spikedet.append_attribute("sed").set_value(spikeDuration);

  xml_node multipliers = document.append_child("multipliers");
  multipliers.append_attribute("on").set_value(frequencyMultipliersOn);
  for (auto e : frequencyMultipliers) {
    xml_node multi = multipliers.append_child("multi");
    multi.append_attribute("f").set_value(e.first);
    multi.append_attribute("mag").set_value(e.second);
  }

  xml_node elko = document.append_child("elko");
  elko.append_child(node_pcdata).set_value(elkoSession.toStdString().c_str());

  if (!file.save_file(filePath.c_str())) {
    logToFileAndConsole("Error while writing file '" << filePath << "'.");
  }
}

void InfoTable::readXML(const string &filePath,
                        DETECTOR_SETTINGS *spikedetSettings,
                        double *spikeDuration) {
  xml_document file;
  xml_parse_result res = file.load_file(filePath.c_str());

  if (!res)
    return;

  xml_node document = file.child("document");

  xml_node browser = document.child("browser");
  position = browser.attribute("position").as_int(position);
  positionIndicator =
      browser.attribute("positionIndicator").as_double(positionIndicator);
  virtualWidth = browser.attribute("virtualWidth").as_int(virtualWidth);

  xml_node filter = document.child("filter");
  filterWindow = static_cast<AlenkaSignal::WindowFunction>(
      browser.attribute("filterWindow").as_int(static_cast<int>(filterWindow)));

  xml_node lowpass = filter.child("lowpass");
  lowpassOn = lowpass.attribute("on").as_bool(lowpassOn);
  lowpassFrequency = lowpass.attribute("f").as_double(lowpassFrequency);

  xml_node highpass = filter.child("highpass");
  highpassOn = highpass.attribute("on").as_bool(highpassOn);
  highpassFrequency = highpass.attribute("f").as_double(highpassFrequency);

  xml_node notch = filter.child("notch");
  notchOn = notch.attribute("on").as_bool(notchOn);
  // notchFrequency = notch.attribute("f").as_double(notchFrequency);

  xml_node selection = document.child("selection");
  selectedMontage =
      selection.attribute("selectedMontage").as_int(selectedMontage);
  selectedType = selection.attribute("selectedType").as_int(selectedType);

  xml_node window = document.child("window");
  timeMode = static_cast<TimeMode>(
      window.attribute("timeMode").as_int(static_cast<int>(timeMode)));
  timeLineInterval =
      window.attribute("timeLineInterval").as_double(timeLineInterval);

  xml_node sample = document.child("sample");
  sampleScale = sample.attribute("scale").as_float(sampleScale);
  sampleUnits = sample.attribute("units").as_int(sampleUnits);

  xml_node spikedet = document.child("spikedet");
  spikedetSettings->m_band_low =
      spikedet.attribute("fl").as_int(spikedetSettings->m_band_low);
  spikedetSettings->m_band_high =
      spikedet.attribute("fh").as_int(spikedetSettings->m_band_high);
  spikedetSettings->m_k1 =
      spikedet.attribute("k1").as_double(spikedetSettings->m_k1);
  spikedetSettings->m_k2 =
      spikedet.attribute("k2").as_double(spikedetSettings->m_k2);
  spikedetSettings->m_k3 =
      spikedet.attribute("k3").as_double(spikedetSettings->m_k3);
  spikedetSettings->m_winsize =
      spikedet.attribute("w").as_int(spikedetSettings->m_winsize);
  spikedetSettings->m_noverlap =
      spikedet.attribute("n").as_double(spikedetSettings->m_noverlap);
  spikedetSettings->m_buffering =
      spikedet.attribute("buf").as_int(spikedetSettings->m_buffering);
  spikedetSettings->m_main_hum_freq =
      spikedet.attribute("h").as_int(spikedetSettings->m_main_hum_freq);
  spikedetSettings->m_discharge_tol =
      spikedet.attribute("dt").as_double(spikedetSettings->m_discharge_tol);
  spikedetSettings->m_polyspike_union_time = spikedet.attribute("pt").as_double(
      spikedetSettings->m_polyspike_union_time);
  spikedetSettings->m_decimation =
      spikedet.attribute("dec").as_int(spikedetSettings->m_decimation);
  *spikeDuration = spikedet.attribute("sed").as_double(*spikeDuration);

  xml_node multipliers = document.child("multipliers");
  frequencyMultipliersOn =
      multipliers.attribute("on").as_bool(frequencyMultipliersOn);
  frequencyMultipliers.clear();
  xml_node multi = multipliers.child("multi");
  while (multi) {
    auto p = make_pair(multi.attribute("f").as_double(),
                       multi.attribute("mag").as_double(1));
    frequencyMultipliers.push_back(p);

    multi = multi.next_sibling("multi");
  }

  xml_node elko = document.child("elko");
  elkoSession =
      QString(elko.text().as_string(elkoSession.toStdString().c_str()));
}
