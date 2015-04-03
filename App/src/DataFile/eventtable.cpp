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

	assert(rowCount() == label.size());
	assert(rowCount() == type.size());
	assert(rowCount() == position.size());
	assert(rowCount() == duration.size());
	assert(rowCount() == channel.size());
	assert(rowCount() == description.size());

	for (unsigned int i = 0; i < label.size(); ++i)
	{
		xml->writeStartElement("event");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("type", QString::number(type[i]));
		xml->writeAttribute("position", QString::number(position[i]));
		xml->writeAttribute("duration", QString::number(duration[i]));
		xml->writeAttribute("channel", QString::number(channel[i]));

		if (description[i].empty() == false)
		{
			xml->writeTextElement("description", QString::fromStdString(description[i]));
		}

		xml->writeEndElement();
	}

	xml->writeEndElement();
}

#define readNumericAttribute(a_, b_)\
	{\
		bool ok;\
		auto tmp = xml->attributes().value(#a_).b_(&ok);\
		(a_).push_back(ok ? tmp : 0);\
	}

void EventTable::read(QXmlStreamReader* xml)
{
	assert(xml->name() == "eventTable");

	while (xml->readNextStartElement())
	{
		if (xml->name() == "event")
		{
			label.push_back(xml->attributes().value("label").toString().toStdString());
			readNumericAttribute(type, toInt);
			readNumericAttribute(position, toInt);
			readNumericAttribute(duration, toInt);
			readNumericAttribute(channel, toInt);

			description.push_back("");
			while (xml->readNextStartElement())
			{
				if (xml->name() == "description")
				{
					description.back() = xml->readElementText().toStdString();
				}
				else
				{
					xml->skipCurrentElement();
				}
			}
		}
		else
		{
			xml->skipCurrentElement();
		}
	}
}

#undef readIntAttribute

void EventTable::getEventsForRendering(int firstSample, int lastSample, vector<tuple<int, int, int>>* allChannelEvents, vector<tuple<int, int, int, int>>* singleChannelEvents)
{
	for (int i = 0; i < rowCount(); ++i)
	{
		if (position[i] <= lastSample && firstSample <= position[i] + duration[i] - 1)
		{
			if (channel[i] < 0)
			{
				allChannelEvents->emplace_back(type[i], position[i], duration[i]);
			}
			else
			{
				singleChannelEvents->emplace_back(type[i], channel[i], position[i], duration[i]);
			}
		}
	}

	std::sort(allChannelEvents->begin(), allChannelEvents->end(), [] (tuple<int, int, int> a, tuple<int, int, int> b) { return get<0>(a) < get<0>(b); });

	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<1>(a) < get<1>(b); });
	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<0>(a) < get<0>(b); });
}

QVariant EventTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return "Label";
		case 1:
			return "Type";
		case 2:
			return "Position";
		case 3:
			return "Duration";
		case 4:
			return "Channel";
		case 5:
			return "Description";
		}
	}

	return QVariant();
}

QVariant EventTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return QString::fromStdString(label[index.row()]);
			case 1:
				return type[index.row()];
			case 2:
				return position[index.row()];
			case 3:
				return duration[index.row()];
			case 4:
				return channel[index.row()];
			case 5:
				return QString::fromStdString(description[index.row()]);
			}
		}
	}

	return QVariant();
}

bool EventTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid())
	{
		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				label[index.row()] = value.toString().toStdString();
				break;
			case 1:
				type[index.row()] = value.value<decltype(type)::value_type>();
				break;
			case 2:
				position[index.row()] = value.value<decltype(position)::value_type>();
				break;
			case 3:
				duration[index.row()] = value.value<decltype(duration)::value_type>();
				break;
			case 4:
				channel[index.row()] = value.value<decltype(channel)::value_type>();
				break;
			case 5:
				description[index.row()] = value.toString().toStdString();
				break;
			}

			emit dataChanged(index, index);
			return true;
		}
	}

	return false;
}

bool EventTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			std::stringstream ss;
			ss << "Event " << row + i;

			label.insert(label.begin() + row + i, ss.str());
			type.insert(type.begin() + row + i, 0); // TODO: load current selected type
			position.insert(position.begin() + row + i, 0);
			duration.insert(duration.begin() + row + i, 1);
			channel.insert(channel.begin() + row + i, -1);
			description.insert(description.begin() + row + i, "");
		}

		endInsertRows();

		return true;
	}
	else
	{
		return false;
	}
}

bool EventTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		int end = row + count;

		label.erase(label.begin() + row, label.begin() + end);
		type.erase(type.begin() + row, type.begin() + end);
		position.erase(position.begin() + row, position.begin() + end);
		duration.erase(duration.begin() + row, duration.begin() + end);
		channel.erase(channel.begin() + row, channel.begin() + end);
		description.erase(description.begin() + row, description.begin() + end);

		endRemoveRows();

		return true;
	}
	else
	{
		return false;
	}
}
