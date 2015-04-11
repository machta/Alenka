#include "infotable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

InfoTable::InfoTable()
{
}

InfoTable::~InfoTable()
{
}

void InfoTable::write(QXmlStreamWriter* xml) const
{
	xml->writeTextElement("virtualWidth", QString::number(virtualWidth));
	xml->writeTextElement("position", QString::number(position));
	xml->writeTextElement("lowpassFrequency", QString::number(lowpassFrequency));
	xml->writeTextElement("highPassFrequency", QString::number(highPassFrequency));
	xml->writeTextElement("notch", notch ? "1" : "0");
	xml->writeTextElement("selectedMontage", QString::number(selectedMontage));
	xml->writeTextElement("timeMode", QString::number(static_cast<int>(timeMode)));
	xml->writeTextElement("selectedType", QString::number(selectedType));
}

#define readNumericElement(a_, b_)\
	if (xml->name() == #a_)\
	{\
		bool ok;\
		auto tmp = xml->readElementText().b_(&ok);\
		(a_) = ok ? tmp : (a_);\
		continue;\
	}

void InfoTable::read(QXmlStreamReader* xml)
{
	while (xml->readNextStartElement())
	{
		readNumericElement(virtualWidth, toInt);
		readNumericElement(position, toInt);
		readNumericElement(lowpassFrequency, toDouble);
		readNumericElement(highPassFrequency, toDouble);

		if (xml->name() == "notch")
		{
			notch = xml->readElementText() == "0" ? false : true;
			continue;
		}

		readNumericElement(selectedMontage, toInt);

		if (xml->name() == "timeMode")
		{
			bool ok;
			int mode = xml->readElementText().toInt(&ok);
			if (ok && 0 <= mode && mode < static_cast<int>(TimeMode::size))
			{
				timeMode = static_cast<TimeMode>(mode);
			}
			continue;
		}

		readNumericElement(selectedType, toInt);

		xml->skipCurrentElement();
	}
}

#undef readNumericElement
