#include "montagetable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

using namespace std;

MontageTable::MontageTable(EventTypeTable* eventTypeTable, QObject* parent)
	: QAbstractTableModel(parent), eventTypeTable(eventTypeTable)
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

			trackTables.push_back(new TrackTable);
			eventTables.push_back(new EventTable(eventTypeTable, trackTables.back()));

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

QVariant MontageTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return "Name";
		case 1:
			return "Save";
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
			switch (index.column())
			{
			case 0:
				return QString::fromStdString(name[row]);
			case 1:
				return save[row];
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
			switch (index.column())
			{
			case 0:
				name[row] = value.toString().toStdString();
				break;
			case 1:
				save[row] = value.toBool();
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
	if (count > 0)
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			std::stringstream ss;
			ss << "Montage " << row + i;

			name.push_back(ss.str());
			save.push_back(false);

			trackTables.push_back(new TrackTable);
			eventTables.push_back(new EventTable(eventTypeTable, trackTables.back()));

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

bool MontageTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row + i];

			name.erase(name.begin() + index);
			save.erase(save.begin() + index);

			trackTables.erase(trackTables.begin() + index);
			eventTables.erase(eventTables.begin() + index);

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

void MontageTable::sort(int column, Qt::SortOrder order)
{
	assert(0 <= column && column < columnCount());

	QCollator collator;
	collator.setNumericMode(true);

	function<bool (int, int)> predicate;

	if (order == Qt::AscendingOrder)
	{
		switch (column)
		{
		case 0:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) < 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return save[a] < save[b]; };
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
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(name[a]), QString::fromStdString(name[b])) > 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return save[a] > save[b]; };
			break;
		default:
			return;
		}
	}

	std::sort(this->order.begin(), this->order.end(), predicate);

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
