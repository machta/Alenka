#include "tracktable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

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

	assert(static_cast<size_t>(rowCount()) == label.size());
	assert(static_cast<size_t>(rowCount()) == code.size());
	assert(static_cast<size_t>(rowCount()) == color.size());
	assert(static_cast<size_t>(rowCount()) == amplitude.size());
	assert(static_cast<size_t>(rowCount()) == hidden.size());

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
			order.push_back(order.size());

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

vector<string> TrackTable::getCode() const
{
	vector<string> newCode;
	for (unsigned int i = 0; i < code.size(); ++i)
	{
		if (hidden[i] == false)
		{
			newCode.push_back(code[i]);
		}
	}
	return newCode;
}

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
		int row = order[index.row()];

		if (role == Qt::DecorationRole)
		{
			switch (index.column())
			{
			case 2:
				return color[row];
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return QString::fromStdString(label[row]);
			case 1:
				return QString::fromStdString(code[row]);
			case 2:
				return color[row];
			case 3:
				return amplitude[row];
			case 4:
				return hidden[row];
			}
		}
	}

	return QVariant();
}

bool TrackTable::setData(const QModelIndex& index, const QVariant &value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				label[row] = value.toString().toStdString();
				break;
			case 1:
				code[row] = value.toString().toStdString();
				break;
			case 2:
				color[row] = value.value<QColor>();
				break;
			case 3:
				amplitude[row] = value.toDouble();
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

			label.push_back(ssLabel.str());
			code.push_back(ssCode.str());
			color.push_back(QColor(Qt::black));
			amplitude.push_back(-0.000008); // TODO: load global amplitude
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

bool TrackTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row + i];

			label.erase(label.begin() + index);
			code.erase(code.begin() + index);
			color.erase(color.begin() + index);
			amplitude.erase(amplitude.begin() + index);
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

void TrackTable::sort(int column, Qt::SortOrder order)
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
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) < 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return code[a] < code[b]; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return color[a].name() < color[b].name(); };
			break;
		case 3:
			predicate = [this] (int a, int b) { return amplitude[a] < amplitude[b]; };
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
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) > 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return code[a] > code[b]; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return color[a].name() > color[b].name(); };
			break;
		case 3:
			predicate = [this] (int a, int b) { return amplitude[a] > amplitude[b]; };
			break;
		case 4:
			predicate = [this] (int a, int b) { return hidden[a] > hidden[b]; };
			break;
		default:
			return;
		}
	}

	std::sort(this->order.begin(), this->order.end(), predicate);

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
