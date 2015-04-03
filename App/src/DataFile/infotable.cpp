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

		xml->skipCurrentElement();
	}
}

#undef readNumericElement
