#include "montagetable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

using namespace std;

MontageTable::MontageTable(QObject* parent) : QAbstractTableModel(parent)
{
}

MontageTable::~MontageTable()
{
	for (const auto& e : trackTables)
	{
		delete e;
	}

	for (const auto& e : eventTables)
	{
		delete e;
	}
}

void MontageTable::write(QXmlStreamWriter* xml) const
{
	assert(static_cast<size_t>(rowCount()) == name.size());
	assert(static_cast<size_t>(rowCount()) == save.size());
	assert(static_cast<size_t>(rowCount()) == trackTables.size());
	assert(static_cast<size_t>(rowCount()) == eventTables.size());

	xml->writeStartElement("montageTable");

	for (int i = 0; i < rowCount(); ++i)
	{
		xml->writeStartElement("montage");

		xml->writeAttribute("name", QString::fromStdString(name[i]));
		xml->writeAttribute("save", save[i] ? "1" : "0");

		trackTables[i]->write(xml);
		eventTables[i]->write(xml);

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

void MontageTable::read(QXmlStreamReader* xml)
{
	assert(xml->name() == "montageTable");

	while (xml->readNextStartElement())
	{
		if (xml->name() == "montage")
		{
			order.push_back(order.size());

			name.push_back(xml->attributes().value("name").toString().toStdString());
			save.push_back(xml->attributes().value("save") == "0" ? false : true);

			pushBackNew();

			while (xml->readNextStartElement())
			{
				if (xml->name() == "trackTable")
				{
					trackTables.back()->read(xml);
				}
				else if (xml->name() == "eventTable")
				{
					eventTables.back()->read(xml);
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

#undef readNumericAttribute

bool MontageTable::insertRowsBack(int count)
{
	assert(count >= 1);

	int row = rowCount();

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; ++i)
	{
		std::stringstream ss;
		ss << "Montage " << row + i;

		name.push_back(ss.str());
		save.push_back(false);

		pushBackNew();

		order.push_back(order.size());
	}

	endInsertRows();

	return true;
}

QVariant MontageTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (static_cast<Column>(section))
		{
		case Column::name:
			return "Name";
		case Column::save:
			return "Save";			
		default:
			break;
		}
	}

	return QVariant();
}

QVariant MontageTable::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (static_cast<Column>(index.column()))
			{
			case Column::name:
				return QString::fromStdString(name[row]);
			case Column::save:
				return save[row];				
			default:
				break;
			}
		}
	}

	return QVariant();
}

bool MontageTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (static_cast<Column>(index.column()))
			{
			case Column::name:
				name[row] = value.toString().toStdString();
				break;
			case Column::save:
				save[row] = value.toBool();
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

bool MontageTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
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

bool MontageTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row];

			name.erase(name.begin() + index);
			save.erase(save.begin() + index);

			trackTables.erase(trackTables.begin() + index);
			eventTables.erase(eventTables.begin() + index);

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

void MontageTable::sort(int column, Qt::SortOrder order)
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
		case Column::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) < 0; };
			break;
		case Column::save:
			predicate = [this] (int a, int b) { return save[a] < save[b]; };
			break;
		default:
			return;
		}
	}
	else
	{
		switch (static_cast<Column>(column))
		{
		case Column::name:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) > 0; };
			break;
		case Column::save:
			predicate = [this] (int a, int b) { return save[a] > save[b]; };
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

void MontageTable::pushBackNew()
{
	TrackTable* tt = new TrackTable;
	EventTable* et = new EventTable;
	tt->setReferences(et);
	et->setReferences(eventTypeTable, tt);
	trackTables.push_back(tt);
	eventTables.push_back(et);
}
