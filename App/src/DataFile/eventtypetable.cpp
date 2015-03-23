#include "eventtypetable.h"

#include <cassert>

using namespace std;

EventTypeTable::EventTypeTable(QObject* parent) : QAbstractTableModel(parent)
{

}

EventTypeTable::~EventTypeTable()
{

}

void EventTypeTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("eventTypeTable");

	assert(name.size() == opacity.size());
	assert(name.size() == color.size());
	assert(name.size() == hidden.size());

	for (int i = 0; i < name.size(); ++i)
	{
		xml->writeStartElement("eventType");

		xml->writeAttribute("name", QString::fromStdString(name[i]));
		xml->writeAttribute("opacity", QString::number(opacity[i]));
		xml->writeAttribute("color", color[i].name());
		xml->writeAttribute("hidden", hidden[i] ? "1" : "0");

		xml->writeEndElement();
	}

	xml->writeEndElement();
}

void EventTypeTable::read(QXmlStreamReader* xml)
{
	while (xml->readNextStartElement() && xml->name() == "eventType")
	{
		name.push_back(xml->attributes().value("name").toString().toStdString());
		opacity.push_back(xml->attributes().value("opacity").toDouble());
		color.push_back(QColor(xml->attributes().value("color").toString()));
		hidden.push_back(xml->attributes().value("hidden") == "0" ? true : false);

		xml->skipCurrentElement();
	}
}

