#include "infotable.h"

#include "../error.h"

#include <pugixml.hpp>

using namespace std;
using namespace pugi;

void InfoTable::writeXML(const string& filePath) const
{
	xml_document file;
	xml_node document = file.append_child("document");

	xml_node browserSettings = document.append_child("browserSettings");
	browserSettings.append_attribute("position").set_value(position);
	browserSettings.append_attribute("positionIndicator").set_value(positionIndicator);
	browserSettings.append_attribute("virtualWidth").set_value(virtualWidth);

	xml_node filter = document.append_child("filter");
	filter.append_attribute("lowpassFrequency").set_value(lowpassFrequency);
	filter.append_attribute("highPassFrequency").set_value(highPassFrequency);
	filter.append_attribute("notch").set_value(notch);

	xml_node selection = document.append_child("selection");
	selection.append_attribute("selectedMontage").set_value(selectedMontage);
	selection.append_attribute("selectedType").set_value(selectedType);

	xml_node windowSettings = document.append_child("windowSettings");
	windowSettings.append_attribute("timeMode").set_value(static_cast<int>(timeMode));
	windowSettings.append_attribute("timeLineInterval").set_value(timeLineInterval);

	if (!file.save_file(filePath.c_str()))
	{
		logToFileAndConsole("Error while writing file '" << filePath << "'.");
	}
}

void InfoTable::readXML(const string& filePath)
{
	xml_document file;
	xml_parse_result res = file.load_file(filePath.c_str());

	if (!res)
		return;

	xml_node document = file.child("document");

	xml_node browserSettings = document.child("browserSettings");
	position = browserSettings.attribute("position").as_int(position);
	positionIndicator = browserSettings.attribute("positionIndicator").as_double(positionIndicator);
	virtualWidth = browserSettings.attribute("virtualWidth").as_int(virtualWidth);

	xml_node filter = document.child("filter");
	lowpassFrequency = filter.attribute("lowpassFrequency").as_double(lowpassFrequency);
	highPassFrequency = filter.attribute("highPassFrequency").as_double(highPassFrequency);
	notch = filter.attribute("notch").as_bool(notch);

	xml_node selection = document.child("selection");
	selectedMontage = selection.attribute("selectedMontage").as_int(selectedMontage);
	selectedType = selection.attribute("selectedType").as_int(selectedType);

	xml_node windowSettings = document.child("windowSettings");
	timeMode = static_cast<TimeMode>(windowSettings.attribute("timeMode").as_int(static_cast<int>(timeMode)));
	timeLineInterval = windowSettings.attribute("timeLineInterval").as_double(timeLineInterval);
}

#undef readNumericElement
