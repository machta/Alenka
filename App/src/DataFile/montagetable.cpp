#include "montagetable.h"

#include <cassert>

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
			name.push_back(xml->attributes().value("name").toString().toStdString());
			save.push_back(xml->attributes().value("save") == "0" ? false : true);

			trackTables.push_back(new TrackTable);
			eventTables.push_back(new EventTable);

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
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return QString::fromStdString(name[index.row()]);
			case 1:
				return save[index.row()];
			}
		}
	}

	return QVariant();
}

bool MontageTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid())
	{
		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				name[index.row()] = value.toString().toStdString();
				break;
			case 1:
				save[index.row()] = value.toBool();
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

			name.insert(name.begin() + row + i, ss.str());
			save.insert(save.begin() + row + i, false);
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

		int end = row + count;

		name.erase(name.begin() + row, name.begin() + end);
		save.erase(save.begin() + row, save.begin() + end);

		endRemoveRows();

		return true;
	}
	else
	{
		return false;
	}
}
