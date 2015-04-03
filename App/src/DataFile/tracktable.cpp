#include "tracktable.h"

#include <cassert>

using namespace std;

TrackTable::TrackTable(QObject* parent) : QAbstractTableModel(parent)
{
}

TrackTable::~TrackTable()
{
}

void TrackTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("trackTable");

	assert(rowCount() == label.size());
	assert(rowCount() == code.size());
	assert(rowCount() == color.size());
	assert(rowCount() == amplitude.size());
	assert(rowCount() == hidden.size());

	for (int i = 0; i < rowCount(); ++i)
	{
		xml->writeStartElement("track");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("color", color[i].name());
		xml->writeAttribute("amplitude", QString::number(amplitude[i]));
		xml->writeAttribute("hidden", hidden[i] ? "1" : "0");

		if (code[i].empty() == false)
		{
			xml->writeTextElement("code", QString::fromStdString(code[i]));
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

void TrackTable::read(QXmlStreamReader* xml)
{
	assert(xml->name() == "trackTable");

	while (xml->readNextStartElement())
	{
		if (xml->name() == "track")
		{
			label.push_back(xml->attributes().value("label").toString().toStdString());
			color.push_back(QColor(xml->attributes().value("color").toString()));
			readNumericAttribute(amplitude, toDouble);
			hidden.push_back(xml->attributes().value("hidden") == "0" ? false : true);

			code.push_back("");
			while (xml->readNextStartElement())
			{
				if (xml->name() == "code")
				{
					code.back() = xml->readElementText().toStdString();
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

QVariant TrackTable::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant TrackTable::data(const QModelIndex& index, int role) const
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

bool TrackTable::setData(const QModelIndex& index, const QVariant &value, int role)
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

bool TrackTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
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
			amplitude.insert(amplitude.begin() + row + i, -0.000008); // TODO: load global amplitude
			hidden.insert(hidden.begin() + row + i, false);
		}

		endInsertRows();

		return true;
	}
	else
	{
		return false;
	}
}

bool TrackTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
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
	else
	{
		return false;
	}
}

