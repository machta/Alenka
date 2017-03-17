#include "infotable.h"

#include "../error.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cassert>

using namespace std;

// TODO: Possibly switch to pugixml.

void InfoTable::writeXML(const string& filePath) const
{
	QFile xmlFile(QString::fromStdString(filePath));
	xmlFile.open(QIODevice::WriteOnly);

	QXmlStreamWriter xml(&xmlFile);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-1);
	xml.writeStartDocument();
	xml.writeStartElement("document");

	xml.writeTextElement("virtualWidth", QString::number(virtualWidth));
	xml.writeTextElement("position", QString::number(position));
	xml.writeTextElement("lowpassFrequency", QString::number(lowpassFrequency));
	xml.writeTextElement("highPassFrequency", QString::number(highPassFrequency));
	xml.writeTextElement("notch", notch ? "true" : "false");
	xml.writeTextElement("selectedMontage", QString::number(selectedMontage));
	xml.writeTextElement("timeMode", QString::number(static_cast<int>(timeMode)));
	xml.writeTextElement("selectedType", QString::number(selectedType));

	xml.writeEndDocument();

	if (xml.hasError())
	{
		logToFileAndConsole("XML error occurred while writing to file '" << filePath << "'");
	}

	xmlFile.close();
}

#define readNumericElement(a_, b_)\
	if (xml.name() == #a_)\
	{\
		bool ok;\
		auto tmp = xml.readElementText().b_(&ok);\
		(a_) = ok ? tmp : (a_);\
		continue;\
	}

void InfoTable::readXML(const string& filePath)
{
	QFile xmlFile(QString::fromStdString(filePath));

	if (xmlFile.exists() == false)
		return;

	xmlFile.open(QIODevice::ReadOnly);

	QXmlStreamReader xml(&xmlFile);
	xml.readNextStartElement();
	assert(xml.name() == "document");

	while (xml.readNextStartElement())
	{
		readNumericElement(virtualWidth, toInt);
		readNumericElement(position, toInt);
		readNumericElement(lowpassFrequency, toDouble);
		readNumericElement(highPassFrequency, toDouble);

		if (xml.name() == "notch")
		{
			notch = xml.readElementText() == "false" ? false : true;
			continue;
		}

		readNumericElement(selectedMontage, toInt);

		if (xml.name() == "timeMode")
		{
			bool ok;
			int mode = xml.readElementText().toInt(&ok);
			if (ok && 0 <= mode && mode < static_cast<int>(TimeMode::size))
			{
				timeMode = static_cast<TimeMode>(mode);
			}
			continue;
		}

		readNumericElement(selectedType, toInt);
		readNumericElement(timeLineInterval, toDouble);
		readNumericElement(positionIndicator, toDouble);

		xml.skipCurrentElement();
	}

	if (xml.hasError())
	{
		logToFileAndConsole("XML error(" << xml.error() << ") while reading file '" << filePath << "': " << xml.errorString().toStdString());
	}

	xmlFile.close();
}

#undef readNumericElement
