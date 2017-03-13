#include "eventtypetablemodel.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Id : public TableColumn
{
public:
	Id(InfoTable* infoTable, DataModel dataModel) : TableColumn("ID", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.eventTypeTable->row(row).id;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel.eventTypeTable->row(row);
			et.id = value.toInt();
			dataModel.eventTypeTable->row(row, et);
			return true;
		}

		return false;
	}
};

class Name : public StringTableColumn
{
public:
	Name(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Name", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.eventTypeTable->row(row).name);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel.eventTypeTable->row(row);
			et.name = value.toString().toStdString();
			dataModel.eventTypeTable->row(row, et);
			return true;
		}

		return false;
	}
};

class Opacity : public TableColumn
{
public:
	Opacity(InfoTable* infoTable, DataModel dataModel) : TableColumn("Opacity", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		double opacity = dataModel.eventTypeTable->row(row).opacity*100;

		if (role == Qt::DisplayRole)
		{
			QLocale locale;
			return locale.toString(opacity, 'f', 2) + "%";
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return opacity;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel.eventTypeTable->row(row);

			bool ok;
			double tmp = value.toDouble(&ok)/100;
			if (ok)
			{
				if (et.opacity == tmp)
					return false;
			}
			else
			{
				QLocale locale;
				tmp = locale.toDouble(value.toString())/100;
				if (et.opacity == tmp)
					return false;
			}

			et.opacity = tmp;
			dataModel.eventTypeTable->row(row, et);
			return true;
		}

		return false;
	}
};

class Color : public ColorTableColumn
{
public:
	Color(InfoTable* infoTable, DataModel dataModel) : ColorTableColumn("Color", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole)
		{
			auto colorArray = dataModel.eventTypeTable->row(row).color;
			QColor color;
			color.setRgb(colorArray[0], colorArray[1], colorArray[2]);
			return color;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel.eventTypeTable->row(row);

			QColor color = value.value<QColor>();
			et.color[0] = color.red();
			et.color[1] = color.green();
			et.color[2] = color.blue();

			dataModel.eventTypeTable->row(row, et);
			return true;
		}

		return false;
	}
};

class Hidden : public BoolTableColumn
{
public:
	Hidden(InfoTable* infoTable, DataModel dataModel) : BoolTableColumn("Hidden", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.eventTypeTable->row(row).hidden;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel.eventTypeTable->row(row);
			et.hidden = value.toBool();
			dataModel.eventTypeTable->row(row, et);
			return true;
		}

		return false;
	}
};

} // namespace

EventTypeTableModel::EventTypeTableModel(InfoTable* infoTable, DataModel dataModel, QObject* parent) : TableModel(infoTable, dataModel, parent)
{
	columns.push_back(new Id(infoTable, dataModel));
	columns.push_back(new Name(infoTable, dataModel));
	columns.push_back(new Opacity(infoTable, dataModel));
	columns.push_back(new Color(infoTable, dataModel));
	columns.push_back(new Hidden(infoTable, dataModel));
}
