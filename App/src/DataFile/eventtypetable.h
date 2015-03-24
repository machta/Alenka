#ifndef EVENTTYPETABLE_H
#define EVENTTYPETABLE_H

#include <QAbstractTableModel>

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>

class EventTypeTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	EventTypeTable(QObject* parent = 0);
	~EventTypeTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return name.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
	{
		return 4;
	}
	virtual QVariant ​headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case 0:
				return QString("Name");
			case 1:
				return QString("Opacity");
			case 2:
				return QString("Color");
			case 3:
				return QString("Hidden");
			}
		}

		return QVariant();
	}
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
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
					return opacity[index.row()];
				case 2:
					return color[index.row()];
				case 3:
					return hidden[index.row()];
				}
			}
		}

		return QVariant();
	}
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole)
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
					opacity[index.row()] = value.toDouble();
					break;
				case 2:
					color[index.row()] = value.value<QColor>();
					break;
				case 3:
					hidden[index.row()] = value.toBool();
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
			ss << "Type " << row;

			name.insert(name.begin() + row + i, ss.str());
			opacity.insert(opacity.begin() + row + i, 0.25);
			color.insert(color.begin() + row + i, QColor(Qt::red));
			hidden.insert(hidden.begin() + row + i, false);
		}

		endInsertRows();

		return true;
	}
	virtual bool ​removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		int end = row + count;

		name.erase(name.begin() + row, name.begin() + end);
		opacity.erase(opacity.begin() + row, opacity.begin() + end);
		color.erase(color.begin() + row, color.begin() + end);
		hidden.erase(hidden.begin() + row, hidden.begin() + end);

		endRemoveRows();

		return true;
	}

private:
	std::vector<std::string> name;
	std::vector<double> opacity;
	std::vector<QColor> color;
	std::vector<bool> hidden;
};

#endif // EVENTTYPETABLE_H
