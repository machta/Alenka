#include "infotable.h"

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
}

#define readNumericElement(a_, b_)\
	if (name == #a_)\
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
		auto name = xml->name();

		readNumericElement(virtualWidth, toInt);
		readNumericElement(position, toInt);
		readNumericElement(lowpassFrequency, toDouble);
		readNumericElement(highPassFrequency, toDouble);

		if (name == "notch")
		{
			notch = xml->readElementText() == "0" ? false : true;
			continue;
		}

		xml->skipCurrentElement();
	}
}

#undef readNumericElement
