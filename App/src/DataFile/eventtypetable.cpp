#include "eventtypetable.h"

#include "montagetable.h"
#include "eventtable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QCollator>

#include <algorithm>
#include <functional>
#include <QLocale>

using namespace std;

EventTypeTable::EventTypeTable(DataFile* file, QObject* parent) : QAbstractTableModel(parent), file(file)
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
		switch (static_cast<Column>(section))
		{
		case Column::id:
			return QString("ID");
		case Column::name:
			return QString("Name");
		case Column::opacity:
			return QString("Opacity");
		case Column::color:
			return QString("Color");
		case Column::hidden:
			return QString("Hidden");
		default:
			break;
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
			switch (static_cast<Column>(index.column()))
			{
			case Column::color:
				return color[row];
			default:
				break;
			}
		}

		if (role == Qt::DisplayRole)
		{
			switch (static_cast<Column>(index.column()))
			{
			case Column::opacity:
			{
				QLocale locale;
				return locale.toString(opacity[row]*100, 'f', 2) + "%";
			}
			default:
				break;
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (static_cast<Column>(index.column()))
			{
			case Column::id:
				return id[row];
			case Column::name:
				return QString::fromStdString(name[row]);
			case Column::opacity:
				return opacity[row]*100;
			case Column::color:
				return color[row];
			case Column::hidden:
				return hidden[row];
			default:
				break;
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
			switch (static_cast<Column>(index.column()))
			{
			case Column::id:
				id[row] = value.value<decltype(id)::value_type>();
				break;
			case Column::name:
				name[row] = value.toString().toStdString();
				break;
			case Column::opacity:
				opacity[row] = value.toDouble()/100;
				break;
			case Column::color:
				color[row] = value.value<QColor>();
				break;
			case Column::hidden:
				hidden[row] = value.toBool();
				break;
			default:
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
				eventTable->emitColumnChanged(EventTable::Column::type);
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
		switch (static_cast<Column>(column))
		{
		case Column::id:
			predicate = [this] (int a, int b) { return id[a] < id[b]; };
			break;
		case Column::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) < 0; };
			break;
		case Column::opacity:
			predicate = [this] (int a, int b) { return opacity[a] < opacity[b]; };
			break;
		case Column::color:
			predicate = [this] (int a, int b) { return color[a].name() < color[b].name(); };
			break;
		case Column::hidden:
			predicate = [this] (int a, int b) { return hidden[a] < hidden[b]; };
			break;
		default:
			return;
		}
	}
	else
	{
		switch (static_cast<Column>(column))
		{
		case Column::id:
			predicate = [this] (int a, int b) { return id[a] > id[b]; };
			break;
		case Column::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) > 0; };
			break;
		case Column::opacity:
			predicate = [this] (int a, int b) { return opacity[a] > opacity[b]; };
			break;
		case Column::color:
			predicate = [this] (int a, int b) { return color[a].name() > color[b].name(); };
			break;
		case Column::hidden:
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
