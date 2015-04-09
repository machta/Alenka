#include "eventtable.h"

#include "eventtypetable.h"
#include "tracktable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

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

	assert(static_cast<size_t>(rowCount()) == label.size());
	assert(static_cast<size_t>(rowCount()) == type.size());
	assert(static_cast<size_t>(rowCount()) == position.size());
	assert(static_cast<size_t>(rowCount()) == duration.size());
	assert(static_cast<size_t>(rowCount()) == channel.size());
	assert(static_cast<size_t>(rowCount()) == description.size());

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
			order.push_back(order.size());

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
		if (position[i] <= lastSample && firstSample <= position[i] + duration[i] - 1 &&
			type[i] >= 0 && getEventTypeTable()->getHidden(type[i]) == false && channel[i] >= -1)
		{
			if (channel[i] == - 1)
			{
				allChannelEvents->emplace_back(type[i], position[i], duration[i]);
			}
			else
			{
				if (getTrackTable()->getHidden(channel[i]) == false)
				{
					singleChannelEvents->emplace_back(type[i], channel[i], position[i], duration[i]);
				}
			}
		}
	}

	std::sort(allChannelEvents->begin(), allChannelEvents->end(), [] (tuple<int, int, int> a, tuple<int, int, int> b) { return get<0>(a) < get<0>(b); });

	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<1>(a) < get<1>(b); });
	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<0>(a) < get<0>(b); });
}

bool EventTable::insertRowsBack(int count)
{
	assert(count >= 1);

	int row = rowCount();

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; ++i)
	{
		std::stringstream ss;
		ss << "Event " << row + i;

		label.push_back(ss.str());
		type.push_back(-1); // TODO: load current selected type
		position.push_back(0);
		duration.push_back(1);
		channel.push_back(-2);
		description.push_back("");

		order.push_back(order.size());
	}

	endInsertRows();

	return true;
}

QVariant EventTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case Collumn::label:
			return "Label";
		case Collumn::type:
			return "Type";
		case Collumn::position:
			return "Position";
		case Collumn::duration:
			return "Duration";
		case Collumn::channel:
			return "Channel";
		case Collumn::description:
			return "Description";
		}
	}

	return QVariant();
}

QVariant EventTable::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
			case Collumn::type:
				return type[row] < 0 ? EventTypeTable::NO_TYPE_STRING : QString::fromStdString(eventTypeTable->getName(type[row]));
			case Collumn::position:
				return position[row]; // TODO
			case Collumn::duration:
				return duration[row]; // TODO
			case Collumn::channel:
				return channel[row] < 0 ?
					   channel[row] == -1 ? TrackTable::ALL_CHANNEL_STRING : TrackTable::NO_CHANNEL_STRING :
					   QString::fromStdString(trackTable->getLabel(channel[row]));
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case Collumn::label:
				return QString::fromStdString(label[row]);
			case Collumn::type:
				return type[row];
			case Collumn::position:
				return position[row];
			case Collumn::duration:
				return duration[row];
			case Collumn::channel:
				return channel[row];
			case Collumn::description:
				return QString::fromStdString(description[row]);
			}
		}
	}

	return QVariant();
}

bool EventTable::setData(const QModelIndex& index, const QVariant &value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case Collumn::label:
				label[row] = value.toString().toStdString();
				break;
			case Collumn::type:
				type[row] = value.value<decltype(type)::value_type>();
				break;
			case Collumn::position:
				position[row] = value.value<decltype(position)::value_type>();
				break;
			case Collumn::duration:
				duration[row] = value.value<decltype(duration)::value_type>();
				break;
			case Collumn::channel:
				channel[row] = value.value<decltype(channel)::value_type>();
				break;
			case Collumn::description:
				description[row] = value.toString().toStdString();
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
	if (count >= 1 && row == rowCount())
	{
		insertRowsBack(count);
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

		for (int i = 0; i < count; ++i)
		{
			int index = order[row];

			label.erase(label.begin() + index);
			type.erase(type.begin() + index);
			position.erase(position.begin() + index);
			duration.erase(duration.begin() + index);
			channel.erase(channel.begin() + index);
			description.erase(description.begin() + index);

			order.erase(order.begin() + row);

			for (auto& e : order)
			{
				if (e > index)
				{
					--e;
				}
			}
		}

		endRemoveRows();

		return true;
	}
	else
	{
		return false;
	}
}

void EventTable::sort(int column, Qt::SortOrder order)
{
	assert(0 <= column && column < columnCount());

	// Build an object for sorting according to the appropriate column.
	QCollator collator;
	collator.setNumericMode(true);

	function<bool (int, int)> predicate;

	if (order == Qt::AscendingOrder)
	{
		switch (column)
		{
		case Collumn::label:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) < 0; };
			break;
		case Collumn::type:
			predicate = [this] (int a, int b) { return type[a] < type[b]; };
			break;
		case Collumn::position:
			predicate = [this] (int a, int b) { return position[a] < position[b]; };
			break;
		case Collumn::duration:
			predicate = [this] (int a, int b) { return duration[a] < duration[b]; };
			break;
		case Collumn::channel:
			predicate = [this] (int a, int b) { return channel[a] < channel[b]; };
			break;
		case Collumn::description:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(description[a]), QString::fromStdString(description[b])) < 0; };
			break;
		default:
			return;
		}
	}
	else
	{
		switch (column)
		{
		case Collumn::label:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) > 0; };
			break;
		case Collumn::type:
			predicate = [this] (int a, int b) { return type[a] > type[b]; };
			break;
		case Collumn::position:
			predicate = [this] (int a, int b) { return position[a] > position[b]; };
			break;
		case Collumn::duration:
			predicate = [this] (int a, int b) { return duration[a] > duration[b]; };
			break;
		case Collumn::channel:
			predicate = [this] (int a, int b) { return channel[a] > channel[b]; };
			break;
		case Collumn::description:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(description[a]), QString::fromStdString(description[b])) > 0; };
			break;
		default:
			return;
		}
	}

	// Update the table.
	emit layoutAboutToBeChanged();

	std::sort(this->order.begin(), this->order.end(), predicate);

	changePersistentIndex(index(0, 0), index(rowCount() - 1, columnCount() - 1));

	emit layoutChanged();
}
