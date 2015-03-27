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

	for (unsigned int i = 0; i < name.size(); ++i)
	{
		xml->writeStartElement("eventType");

		xml->writeAttribute("id", QString::number(id[i]));
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
		id.push_back(xml->attributes().value("id").toInt());
		name.push_back(xml->attributes().value("name").toString().toStdString());
		opacity.push_back(xml->attributes().value("opacity").toDouble());
		color.push_back(QColor(xml->attributes().value("color").toString()));
		hidden.push_back(xml->attributes().value("hidden") == "0" ? true : false);

		xml->skipCurrentElement();
	}
}

QVariant EventTypeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return QString("ID");
		case 1:
			return QString("Name");
		case 2:
			return QString("Opacity");
		case 3:
			return QString("Color");
		case 4:
			return QString("Hidden");
		}
	}

	return QVariant();
}

QVariant EventTypeTable::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return id[index.row()];
			case 1:
				return QString::fromStdString(name[index.row()]);
			case 2:
				return opacity[index.row()];
			case 3:
				return color[index.row()];
			case 4:
				return hidden[index.row()];
			}
		}
	}

	return QVariant();
}

bool EventTypeTable::setData(const QModelIndex &index, const QVariant& value, int role)
{
	if (index.isValid())
	{
		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				id[index.row()] = value.toInt();
				break;
			case 1:
				name[index.row()] = value.toString().toStdString();
				break;
			case 2:
				opacity[index.row()] = value.toDouble();
				break;
			case 3:
				color[index.row()] = value.value<QColor>();
				break;
			case 4:
				hidden[index.row()] = value.toBool();
				break;
			}

			emit dataChanged(index, index);
			return true;
		}
	}

	return false;
}

bool EventTypeTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; ++i)
	{
		std::stringstream ss;
		ss << "Type " << row;

		id.insert(id.begin() + row + i, row + 256*256);
		name.insert(name.begin() + row + i, ss.str());
		opacity.insert(opacity.begin() + row + i, 0.25);
		color.insert(color.begin() + row + i, QColor(Qt::red));
		hidden.insert(hidden.begin() + row + i, false);
	}

	endInsertRows();

	return true;
}

bool EventTypeTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	beginRemoveRows(QModelIndex(), row, row + count - 1);

	int end = row + count;

	id.erase(id.begin() + row, id.begin() + end);
	name.erase(name.begin() + row, name.begin() + end);
	opacity.erase(opacity.begin() + row, opacity.begin() + end);
	color.erase(color.begin() + row, color.begin() + end);
	hidden.erase(hidden.begin() + row, hidden.begin() + end);

	endRemoveRows();

	return true;
}

