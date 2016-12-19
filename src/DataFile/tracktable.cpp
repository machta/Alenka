#include "tracktable.h"

#include "eventtable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QLocale>
#include <QCollator>

#include <algorithm>
#include <functional>

using namespace std;

TrackTable::TrackTable(DataFile* file, QObject* parent) : QAbstractTableModel(parent), file(file)
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

bool TrackTable::insertRowsBack(int count)
{
	assert(count >= 1);

	int row = rowCount();

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

		order.push_back(order.size());
	}

	endInsertRows();

	return true;
}

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
		switch (static_cast<Column>(section))
		{
		case Column::label:
			return "Label";
		case Column::code:
			return "Code";
		case Column::color:
			return "Color";
		case Column::amplitude:
			return "Amplitude";
		case Column::hidden:
			return "Hidden";
		default:
			break;
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
			switch (static_cast<Column>(index.column()))
			{
			case Column::color:
				return color[row];
			default:
				break;
			}
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (static_cast<Column>(index.column()))
			{
			case Column::label:
				return QString::fromStdString(label[row]);
			case Column::code:
				return QString::fromStdString(code[row]);
			case Column::color:
				return color[row];
			case Column::amplitude:
				return amplitude[row];
			case Column::hidden:
				return hidden[row];
			default:
				break;
			}
		}
	}

	return QVariant();
}

#define CASE(a_, b_)\
	case Column::a_:\
		if (a_[row] == (b_)) {	return false; }\
		a_[row] = (b_);\
		break

bool TrackTable::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		QLocale locale;
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (static_cast<Column>(index.column()))
			{
				CASE(label, value.toString().toStdString());
			case Column::code:
				if (code[row] == value.toString().toStdString())
				{
					return false;
				}
				else
				{
					QString c = value.toString();
					if (validateTrackCode(c))
					{
						code[row] = c.toStdString();
						break;
					}
					else
					{
						return false;
					}
				}
				CASE(color, value.value<QColor>());
			case Column::amplitude:
			{
				bool ok;
				double tmp = value.toDouble(&ok);
				if (ok)
				{
					if (amplitude[row] == tmp)
					{
						return false;
					}
					amplitude[row] = tmp;
					break;
				}
				else
				{
					tmp = locale.toDouble(value.toString());
					if (amplitude[row] == tmp)
					{
						return false;
					}
					amplitude[row] = tmp;
					break;
				}
			}
				CASE(hidden, value.toBool());
			default:
				break;
			}

			emit dataChanged(index, index);
			return true;
		}
	}

	return false;
}

#undef CASE

bool TrackTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
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

bool TrackTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		int rowLast = row + count - 1;

		// Update the channels of events to point to correct tracks after the rows are removed.
		vector<int> indexesToBeRemoved;
		for (int i = row; i <= rowLast; ++i)
		{
			indexesToBeRemoved.push_back(order[i]);
		}
		std::sort(indexesToBeRemoved.begin(), indexesToBeRemoved.end());

		bool changed = false;

		for (int j = 0; j < eventTable->rowCount(); ++j)
		{
			int channel = eventTable->getChannel(j);

			if (channel >= 0)
			{
				auto it = lower_bound(indexesToBeRemoved.begin(), indexesToBeRemoved.end(), channel);

				if (it != indexesToBeRemoved.end() && *it == channel)
				{
					eventTable->setChannel(-2, j);
					changed = true;
				}
				else
				{
					channel -= distance(indexesToBeRemoved.begin(), it);
					eventTable->setChannel(channel, j);
				}
			}
		}

		if (changed)
		{
			eventTable->emitColumnChanged(EventTable::Column::channel);
		}

		// Remove rows.
		beginRemoveRows(QModelIndex(), row, rowLast);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row];

			label.erase(label.begin() + index);
			code.erase(code.begin() + index);
			color.erase(color.begin() + index);
			amplitude.erase(amplitude.begin() + index);
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

void TrackTable::sort(int column, Qt::SortOrder order)
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
		case Column::label:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) < 0; };
			break;
		case Column::code:
			predicate = [this] (int a, int b) { return code[a] < code[b]; };
			break;
		case Column::color:
			predicate = [this] (int a, int b) { return color[a].name() < color[b].name(); };
			break;
		case Column::amplitude:
			predicate = [this] (int a, int b) { return amplitude[a] < amplitude[b]; };
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
		case Column::label:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) > 0; };
			break;
		case Column::code:
			predicate = [this] (int a, int b) { return code[a] > code[b]; };
			break;
		case Column::color:
			predicate = [this] (int a, int b) { return color[a].name() > color[b].name(); };
			break;
		case Column::amplitude:
			predicate = [this] (int a, int b) { return amplitude[a] > amplitude[b]; };
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

const char* const TrackTable::NO_CHANNEL_STRING = "<No Channel>";
const char* const TrackTable::ALL_CHANNEL_STRING = "<All>";
