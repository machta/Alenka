#include "eventtypetable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

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

	assert(static_cast<size_t>(rowCount()) == name.size());
	assert(static_cast<size_t>(rowCount()) == id.size());
	assert(static_cast<size_t>(rowCount()) == opacity.size());
	assert(static_cast<size_t>(rowCount()) == color.size());
	assert(static_cast<size_t>(rowCount()) == hidden.size());

	for (int i = 0; i < rowCount(); ++i)
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

#define readNumericAttribute(a_, b_)\
	{\
		bool ok;\
		auto tmp = xml->attributes().value(#a_).b_(&ok);\
		(a_).push_back(ok ? tmp : 0);\
	}

void EventTypeTable::read(QXmlStreamReader* xml)
{
	assert(xml->name() == "eventTypeTable");

	while (xml->readNextStartElement())
	{
		if (xml->name() == "eventType")
		{
			order.push_back(order.size());

			readNumericAttribute(id, toInt);
			name.push_back(xml->attributes().value("name").toString().toStdString());
			readNumericAttribute(opacity, toDouble);
			color.push_back(QColor(xml->attributes().value("color").toString()));
			hidden.push_back(xml->attributes().value("hidden") == "0" ? false : true);
		}

		xml->skipCurrentElement();
	}
}

#undef readNumericAttribute

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
		int row = order[index.row()];

		if (role == Qt::DecorationRole)
		{
			switch (index.column())
			{
			case 3:
				return color[row];
			}
		}

		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
			case 2:
				return QString::number(opacity[row]*100, 'f', 2) + "%";
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return id[row];
			case 1:
				return QString::fromStdString(name[row]);
			case 2:
				return opacity[row]*100;
			case 3:
				return color[row];
			case 4:
				return hidden[row];
			}
		}
	}

	return QVariant();
}

bool EventTypeTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				id[row] = value.value<decltype(id)::value_type>();
				break;
			case 1:
				name[row] = value.toString().toStdString();
				break;
			case 2:
				opacity[row] = value.toDouble()/100;
				break;
			case 3:
				color[row] = value.value<QColor>();
				break;
			case 4:
				hidden[row] = value.toBool();
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
	if (count > 0)
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			std::stringstream ss;
			ss << "Type " << row;

			id.push_back(row);
			name.push_back(ss.str());
			opacity.push_back(0.25);
			color.push_back(QColor(Qt::red));
			hidden.push_back(false);

			order.insert(order.begin() + row + i, order.size());
		}

		endInsertRows();

		return true;
	}
	else
	{
		return false;
	}
}

bool EventTypeTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row + i];

			id.erase(id.begin() + index);
			name.erase(name.begin() + index);
			opacity.erase(opacity.begin() + index);
			color.erase(color.begin() + index);
			hidden.erase(hidden.begin() + index);

			order.erase(order.begin() + row + i);

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

void EventTypeTable::sort(int column, Qt::SortOrder order)
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
		case 0:
			predicate = [this] (int a, int b) { return id[a] < id[b]; };
			break;
		case 1:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) < 0; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return opacity[a] < opacity[b]; };
			break;
		case 3:
			predicate = [this] (int a, int b) { return color[a].name() < color[b].name(); };
			break;
		case 4:
			predicate = [this] (int a, int b) { return hidden[a] < hidden[b]; };
			break;
		default:
			return;
		}
	}
	else
	{
		switch (column)
		{
		case 0:
			predicate = [this] (int a, int b) { return id[a] > id[b]; };
			break;
		case 1:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) > 0; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return opacity[a] > opacity[b]; };
			break;
		case 3:
			predicate = [this] (int a, int b) { return color[a].name() > color[b].name(); };
			break;
		case 4:
			predicate = [this] (int a, int b) { return hidden[a] > hidden[b]; };
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
