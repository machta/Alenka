#ifndef MONTAGETABLE_H
#define MONTAGETABLE_H

#include <QAbstractTableModel>

#include "eventtable.h"

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>

class MontageTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	MontageTable(QObject* parent = 0);
	~MontageTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
	{
		return 5;
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
			std::stringstream ssLabel, ssCode;
			ssLabel << "Type " << row;
			ssCode << "out = in(" << row << ");";

			label.insert(label.begin() + row + i, ssLabel.str());
			code.insert(code.begin() + row + i, ssCode.str());
			color.insert(color.begin() + row + i, QColor(Qt::black));
			amplitude.insert(amplitude.begin() + row + i, 1); // TODO: load global amplitude
			hidden.insert(hidden.begin() + row + i, false);
		}

		endInsertRows();

		return true;
	}
	virtual bool ​removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
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

private:
	std::vector<std::string> label;
	std::vector<std::string> code;
	std::vector<QColor> color;
	std::vector<double> amplitude;
	std::vector<bool> hidden;

	EventTable eventTable;
};

#endif // MONTAGETABLE_H

