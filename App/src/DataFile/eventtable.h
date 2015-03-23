#ifndef EVENTTABLE_H
#define EVENTTABLE_H

#include <QAbstractTableModel>

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>

class EventTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	EventTable(QObject* parent = 0);
	~EventTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
	{
		return 6;
	}
	virtual QVariant ​headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case 0:
				return "Label";
			case 1:
				return "Type";
			case 2:
				return "Position";
			case 3:
				return "Duration";
			case 4:
				return "Channel";
			case 5:
				return "Description";
			}
		}

		return QVariant();
	}
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
	{
		if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
		{
			if (role == Qt::DisplayRole)
			{
				switch (index.column())
				{
				case 0:
					return QString::fromStdString(label[index.row()]);
				case 1:
					return type[index.row()];
				case 2:
					return position[index.row()];
				case 3:
					return duration[index.row()];
				case 4:
					return channel[index.row()];
				case 5:
					return QString::fromStdString(description[index.row()]);
				}
			}
		}

		return QVariant();
	}
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole)
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				switch (index.column())
				{
				case 0:
					label[index.row()] = value.toString().toStdString();
					break;
				case 1:
					type[index.row()] = value.toInt();
					break;
				case 2:
					position[index.row()] = value.toDouble();
					break;
				case 3:
					duration[index.row()] = value.toDouble();
					break;
				case 4:
					channel[index.row()] = value.toInt();
					break;
				case 5:
					description[index.row()] = value.toString().toStdString();
					break;
				}

				emit dataChanged(index, index);
				return true;
			}
		}

		return false;
	}
	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (!index.isValid())
		{
			return Qt::ItemIsEnabled;
		}

		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	virtual bool ​insertRows(int row, int count, const QModelIndex& parent = QModelIndex())
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			std::stringstream ss;
			ss << "Event " << row;

			label.insert(label.begin() + row + i, ss.str());
			type.insert(type.begin() + row + i, 0); // TODO: load current selected type
			position.insert(position.begin() + row + i, 0);
			duration.insert(duration.begin() + row + i, 1);
			channel.insert(channel.begin() + row + i, -1);
			description.insert(description.begin() + row + i, "");
		}

		endInsertRows();

		return true;
	}
	virtual bool ​removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		int end = row + count;

		label.erase(label.begin() + row, label.begin() + end);
		type.erase(type.begin() + row, type.begin() + end);
		position.erase(position.begin() + row, position.begin() + end);
		duration.erase(duration.begin() + row, duration.begin() + end);
		channel.erase(channel.begin() + row, channel.begin() + end);
		description.erase(description.begin() + row, description.begin() + end);

		endRemoveRows();

		return true;
	}

private:
	std::vector<std::string> label;
	std::vector<int> type;
	std::vector<qlonglong> position;
	std::vector<qlonglong> duration;
	std::vector<int> channel;
	std::vector<std::string> description;
};

#endif // EVENTTABLE_H
