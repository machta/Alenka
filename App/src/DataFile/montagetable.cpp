#include "montagetable.h"

#include <cassert>

using namespace std;

MontageTable::MontageTable(QObject* parent) : QAbstractTableModel(parent)
{

}

MontageTable::~MontageTable()
{

}

void MontageTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("montageTable");

	assert(label.size() == code.size());
	assert(label.size() == color.size());
	assert(label.size() == amplitude.size());
	assert(label.size() == hidden.size());

	for (int i = 0; i < label.size(); ++i)
	{
		xml->writeStartElement("track");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("color", color[i].name());
		xml->writeAttribute("amplitude", QString::number(amplitude[i]));
		xml->writeAttribute("hidden", hidden[i] ? "1" : "0");

		xml->writeTextElement("code", QString::fromStdString(code[i]));

		xml->writeEndElement();
	}

	eventTable.write(xml);

	xml->writeEndElement();
}

void MontageTable::read(QXmlStreamReader* xml)
{
	while (xml->readNextStartElement())
	{
		auto name = xml->name();
		if (name == "track")
		{
			label.push_back(xml->attributes().value("label").toString().toStdString());
			color.push_back(QColor(xml->attributes().value("color").toString()));
			amplitude.push_back(xml->attributes().value("amplitude").toDouble());
			hidden.push_back(xml->attributes().value("hidden") == "0" ? true : false);

			xml->readNextStartElement();
			assert(xml->name() == "code");
			code.push_back(xml->readElementText().toStdString());

			xml->skipCurrentElement();
		}
		else if (xml->name() == "eventTable")
		{
			eventTable.read(xml);
		}
		else
		{
			xml->skipCurrentElement();
		}
	}
}

