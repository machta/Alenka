#include "eventtable.h"

#include <cassert>

using namespace std;

EventTable::EventTable(QObject* parent) : QAbstractTableModel(parent)
{

}

EventTable::~EventTable()
{

}

void EventTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("eventTable");

	assert(label.size() == type.size());
	assert(label.size() == position.size());
	assert(label.size() == duration.size());
	assert(label.size() == channel.size());
	assert(label.size() == description.size());

	for (int i = 0; i < label.size(); ++i)
	{
		xml->writeStartElement("event");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("type", QString::number(type[i]));
		xml->writeAttribute("position", QString::number(position[i]));
		xml->writeAttribute("duration", QString::number(duration[i]));
		xml->writeAttribute("channel", QString::number(channel[i]));

		xml->writeTextElement("description", QString::fromStdString(description[i]));

		xml->writeEndElement();
	}

	xml->writeEndElement();
}

void EventTable::read(QXmlStreamReader* xml)
{
	while (xml->readNextStartElement() && xml->name() == "event")
	{
		label.push_back(xml->attributes().value("label").toString().toStdString());
		type.push_back(xml->attributes().value("type").toInt());
		position.push_back(xml->attributes().value("position").toLongLong());
		duration.push_back(xml->attributes().value("duration").toLongLong());
		channel.push_back(xml->attributes().value("channel").toInt());

		xml->readNextStartElement();
		assert(xml->name() == "description");
		description.push_back(xml->readElementText().toStdString());

		xml->skipCurrentElement();
	}
}
