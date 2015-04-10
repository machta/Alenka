#include "eventtypetable.h"

#include "montagetable.h"
#include "eventtable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>
#include <vector>

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

bool EventTypeTable::insertRowsBack(int count)
{
	assert(count >= 1);

	int row = rowCount();

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; ++i)
	{
		std::stringstream ss;
		ss << "Type " << row + i + 1;

		id.push_back(row + i + 1);
		name.push_back(ss.str());
		opacity.push_back(0.25);
		color.push_back(QColor(Qt::red));
		hidden.push_back(false);

		order.push_back(order.size());
	}

	endInsertRows();

	return true;
}

QVariant EventTypeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case Collumn::id:
			return QString("ID");
		case Collumn::name:
			return QString("Name");
		case Collumn::opacity:
			return QString("Opacity");
		case Collumn::color:
			return QString("Color");
		case Collumn::hidden:
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
			case Collumn::color:
				return color[row];
			}
		}

		if (role == Qt::DisplayRole)
		{
			switch (index.column())
			{
			case Collumn::opacity:
				return QString::number(opacity[row]*100, 'f', 2) + "%";
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case Collumn::id:
				return id[row];
			case Collumn::name:
				return QString::fromStdString(name[row]);
			case Collumn::opacity:
				return opacity[row]*100;
			case Collumn::color:
				return color[row];
			case Collumn::hidden:
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
			case Collumn::id:
				id[row] = value.value<decltype(id)::value_type>();
				break;
			case Collumn::name:
				name[row] = value.toString().toStdString();
				break;
			case Collumn::opacity:
				opacity[row] = value.toDouble()/100;
				break;
			case Collumn::color:
				color[row] = value.value<QColor>();
				break;
			case Collumn::hidden:
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

bool EventTypeTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		int rowLast = row + count - 1;

		// Update the types of events to point to correct event types after the rows are removed.
		vector<int> indexesToBeRemoved;
		for (int i = row; i <= rowLast; ++i)
		{
			indexesToBeRemoved.push_back(order[i]);
		}
		std::sort(indexesToBeRemoved.begin(), indexesToBeRemoved.end());

		for (int i = 0; i < montageTable->rowCount(); ++i)
		{
			EventTable* eventTable = montageTable->getEventTables()->at(i);

			bool changed = false;

			for (int j = 0; j < eventTable->rowCount(); ++j)
			{
				int type = eventTable->getType(j);

				if (type >= 0)
				{
					auto it = lower_bound(indexesToBeRemoved.begin(), indexesToBeRemoved.end(), type);

					if (it != indexesToBeRemoved.end() && *it == type)
					{
						eventTable->setType(-1, j);
						changed = true;
					}
					else
					{
						type -= distance(indexesToBeRemoved.begin(), it);
						eventTable->setType(type, j);
					}
				}
			}

			if (changed)
			{
				emit eventTable->dataChanged(eventTable->index(0, static_cast<int>(EventTable::Collumn::type)), eventTable->index(eventTable->rowCount() - 1, static_cast<int>(EventTable::Collumn::type)));
			}
		}

		// Remove rows.
		beginRemoveRows(QModelIndex(), row, rowLast);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row];

			id.erase(id.begin() + index);
			name.erase(name.begin() + index);
			opacity.erase(opacity.begin() + index);
			color.erase(color.begin() + index);
			hidden.erase(hidden.begin() + index);

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
		case Collumn::id:
			predicate = [this] (int a, int b) { return id[a] < id[b]; };
			break;
		case Collumn::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) < 0; };
			break;
		case Collumn::opacity:
			predicate = [this] (int a, int b) { return opacity[a] < opacity[b]; };
			break;
		case Collumn::color:
			predicate = [this] (int a, int b) { return color[a].name() < color[b].name(); };
			break;
		case Collumn::hidden:
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
		case Collumn::id:
			predicate = [this] (int a, int b) { return id[a] > id[b]; };
			break;
		case Collumn::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) > 0; };
			break;
		case Collumn::opacity:
			predicate = [this] (int a, int b) { return opacity[a] > opacity[b]; };
			break;
		case Collumn::color:
			predicate = [this] (int a, int b) { return color[a].name() > color[b].name(); };
			break;
		case Collumn::hidden:
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

const char* EventTypeTable::NO_TYPE_STRING = "<No Type>";
