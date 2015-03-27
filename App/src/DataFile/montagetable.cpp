#include "montagetable.h"

#include <cassert>

using namespace std;

MontageTable::MontageTable(QObject* parent) : QAbstractTableModel(parent)
{
}

MontageTable::~MontageTable()
{
}

void MontageTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("montageTable");

	assert(label.size() == code.size());
	assert(label.size() == color.size());
	assert(label.size() == amplitude.size());
	assert(label.size() == hidden.size());

	for (unsigned int i = 0; i < label.size(); ++i)
	{
		xml->writeStartElement("track");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("color", color[i].name());
		xml->writeAttribute("amplitude", QString::number(amplitude[i]));
		xml->writeAttribute("hidden", hidden[i] ? "1" : "0");

		xml->writeTextElement("code", QString::fromStdString(code[i]));

		xml->writeEndElement();
	}

	eventTable.write(xml);

	xml->writeEndElement();
}

void MontageTable::read(QXmlStreamReader* xml)
{
	while (xml->readNextStartElement())
	{
		auto name = xml->name();
		if (name == "track")
		{
			label.push_back(xml->attributes().value("label").toString().toStdString());
			color.push_back(QColor(xml->attributes().value("color").toString()));
			amplitude.push_back(xml->attributes().value("amplitude").toDouble());
			hidden.push_back(xml->attributes().value("hidden") == "0" ? true : false);

			xml->readNextStartElement();
			assert(xml->name() == "code");
			code.push_back(xml->readElementText().toStdString());

			xml->skipCurrentElement();
		}
		else if (xml->name() == "eventTable")
		{
			eventTable.read(xml);
		}
		else
		{
			xml->skipCurrentElement();
		}
	}
}

QVariant MontageTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return "Label";
		case 1:
			return "Code";
		case 2:
			return "Color";
		case 3:
			return "Amplitude";
		case 4:
			return "Hidden";
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
				return QString::fromStdString(label[index.row()]);
			case 1:
				return QString::fromStdString(code[index.row()]);
			case 2:
				return color[index.row()];
			case 3:
				return amplitude[index.row()];
			case 4:
				return hidden[index.row()];
			}
		}
	}

	return QVariant();
}

bool MontageTable::setData(const QModelIndex& index, const QVariant &value, int role)
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
				code[index.row()] = value.toString().toStdString();
				break;
			case 2:
				color[index.row()] = value.value<QColor>();
				break;
			case 3:
				amplitude[index.row()] = value.toDouble();
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

bool MontageTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; ++i)
	{
		std::stringstream ssLabel, ssCode;
		ssLabel << "Track " << row + i;
		ssCode << "out = in(" << row + i << ");";

		label.insert(label.begin() + row + i, ssLabel.str());
		code.insert(code.begin() + row + i, ssCode.str());
		color.insert(color.begin() + row + i, QColor(Qt::black));
		amplitude.insert(amplitude.begin() + row + i, 1); // TODO: load global amplitude
		hidden.insert(hidden.begin() + row + i, false);
	}

	endInsertRows();

	return true;
}

bool MontageTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	beginRemoveRows(QModelIndex(), row, row + count - 1);

	int end = row + count;

	label.erase(label.begin() + row, label.begin() + end);
	code.erase(code.begin() + row, code.begin() + end);
	color.erase(color.begin() + row, color.begin() + end);
	amplitude.erase(amplitude.begin() + row, amplitude.begin() + end);
	hidden.erase(hidden.begin() + row, hidden.begin() + end);

	endRemoveRows();

	return true;
}

