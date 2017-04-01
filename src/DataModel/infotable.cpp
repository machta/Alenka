#include "infotable.h"

#include "../error.h"
#include <AlenkaSignal/spikedet.h>

#include <pugixml.hpp>

using namespace std;
using namespace pugi;

void InfoTable::writeXML(const string& filePath, const AlenkaSignal::DETECTOR_SETTINGS& spikedetSettings, double spikeDuration) const
{
	xml_document file;
	xml_node document = file.append_child("document");

	xml_node browser = document.append_child("browser");
	browser.append_attribute("position").set_value(position);
	browser.append_attribute("positionIndicator").set_value(positionIndicator);
	browser.append_attribute("virtualWidth").set_value(virtualWidth);

	xml_node filter = document.append_child("filter");
	filter.append_attribute("lowpassFrequency").set_value(lowpassFrequency);
	filter.append_attribute("highPassFrequency").set_value(highPassFrequency);
	filter.append_attribute("notch").set_value(notch);
	filter.append_attribute("filterWindow").set_value(static_cast<int>(filterWindow));

	xml_node selection = document.append_child("selection");
	selection.append_attribute("selectedMontage").set_value(selectedMontage);
	selection.append_attribute("selectedType").set_value(selectedType);

	xml_node window = document.append_child("window");
	window.append_attribute("timeMode").set_value(static_cast<int>(timeMode));
	window.append_attribute("timeLineInterval").set_value(timeLineInterval);

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
	spikedet.append_attribute("pt").set_value(spikedetSettings.m_polyspike_union_time);
	spikedet.append_attribute("dec").set_value(spikedetSettings.m_decimation);
	spikedet.append_attribute("sed").set_value(spikeDuration);

	xml_node multipliers = document.append_child("multipliers");
	multipliers.append_attribute("on").set_value(frequencyMultipliersOn);
	for (auto e : frequencyMultipliers)
	{
		xml_node multi = multipliers.append_child("multi");
		multi.append_attribute("f").set_value(e.first);
		multi.append_attribute("mag").set_value(e.second);
	}

	if (!file.save_file(filePath.c_str()))
	{
		logToFileAndConsole("Error while writing file '" << filePath << "'.");
	}
}

void InfoTable::readXML(const string& filePath, AlenkaSignal::DETECTOR_SETTINGS* spikedetSettings, double* spikeDuration)
{
	xml_document file;
	xml_parse_result res = file.load_file(filePath.c_str());

	if (!res)
		return;

	xml_node document = file.child("document");

	xml_node browser = document.child("browser");
	position = browser.attribute("position").as_int(position);
	positionIndicator = browser.attribute("positionIndicator").as_double(positionIndicator);
	virtualWidth = browser.attribute("virtualWidth").as_int(virtualWidth);

	xml_node filter = document.child("filter");
	lowpassFrequency = filter.attribute("lowpassFrequency").as_double(lowpassFrequency);
	highPassFrequency = filter.attribute("highPassFrequency").as_double(highPassFrequency);
	notch = filter.attribute("notch").as_bool(notch);
	filterWindow = static_cast<AlenkaSignal::WindowFunction>(browser.attribute("filterWindow").as_int(static_cast<int>(filterWindow)));

	xml_node selection = document.child("selection");
	selectedMontage = selection.attribute("selectedMontage").as_int(selectedMontage);
	selectedType = selection.attribute("selectedType").as_int(selectedType);

	xml_node window = document.child("window");
	timeMode = static_cast<TimeMode>(window.attribute("timeMode").as_int(static_cast<int>(timeMode)));
	timeLineInterval = window.attribute("timeLineInterval").as_double(timeLineInterval);

	xml_node spikedet = document.child("spikedet");
	spikedetSettings->m_band_low = spikedet.attribute("fl").as_int(spikedetSettings->m_band_low);
	spikedetSettings->m_band_high = spikedet.attribute("fh").as_int(spikedetSettings->m_band_high);
	spikedetSettings->m_k1 = spikedet.attribute("k1").as_double(spikedetSettings->m_k1);
	spikedetSettings->m_k2 = spikedet.attribute("k2").as_double(spikedetSettings->m_k2);
	spikedetSettings->m_k3 = spikedet.attribute("k3").as_double(spikedetSettings->m_k3);
	spikedetSettings->m_winsize = spikedet.attribute("w").as_int(spikedetSettings->m_winsize);
	spikedetSettings->m_noverlap = spikedet.attribute("n").as_double(spikedetSettings->m_noverlap);
	spikedetSettings->m_buffering = spikedet.attribute("buf").as_int(spikedetSettings->m_buffering);
	spikedetSettings->m_main_hum_freq = spikedet.attribute("h").as_int(spikedetSettings->m_main_hum_freq);
	spikedetSettings->m_discharge_tol = spikedet.attribute("dt").as_double(spikedetSettings->m_discharge_tol);
	spikedetSettings->m_polyspike_union_time = spikedet.attribute("pt").as_double(spikedetSettings->m_polyspike_union_time);
	spikedetSettings->m_decimation = spikedet.attribute("dec").as_int(spikedetSettings->m_decimation);
	*spikeDuration = spikedet.attribute("sed").as_double(*spikeDuration);

	xml_node multipliers = document.child("multipliers");
	frequencyMultipliersOn = multipliers.attribute("on").as_bool(frequencyMultipliersOn);
	frequencyMultipliers.clear();
	xml_node multi = multipliers.child("multi");
	while (multi)
	{
		auto p = make_pair(multi.attribute("f").as_double(), multi.attribute("mag").as_double(1));
		frequencyMultipliers.push_back(p);

		multi = multi.next_sibling("multi");
	}
}
