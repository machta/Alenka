#include "eventtypetablemodel.h"

#include "../signalfilebrowserwindow.h"
#include "../DataModel/vitnessdatamodel.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Id : public TableColumn
{
public:
	Id(DataModel* dataModel) : TableColumn("ID", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return dataModel->eventTypeTable()->row(row).id;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel->eventTypeTable()->row(row);
			et.id = value.toInt();
			dataModel->eventTypeTable()->row(row, et);
			return true;
		}

		return false;
	}
};

class Name : public TableColumn
{
public:
	Name(DataModel* dataModel) : TableColumn("Name", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(dataModel->eventTypeTable()->row(row).name);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel->eventTypeTable()->row(row);
			et.name = value.toString().toStdString();
			dataModel->eventTypeTable()->row(row, et);
			return true;
		}

		return false;
	}
};

class Opacity : public TableColumn
{
public:
	Opacity(DataModel* dataModel) : TableColumn("Opacity", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		double opacity = dataModel->eventTypeTable()->row(row).opacity*100;

		if (role == Qt::DisplayRole)
		{
			QLocale locale;
			return locale.toString(opacity, 'f', 2) + "%";
		}
		else if (role == Qt::EditRole)
		{
			return opacity;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel->eventTypeTable()->row(row);

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
			dataModel->eventTypeTable()->row(row, et);
			return true;
		}

		return false;
	}
};

class Color : public ColorTableColumn
{
public:
	Color(DataModel* dataModel) : ColorTableColumn("Color", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole)
		{
			auto colorArray = dataModel->eventTypeTable()->row(row).color;
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
			EventType et = dataModel->eventTypeTable()->row(row);
			DataModel::color2array(value.value<QColor>(), et.color);
			dataModel->eventTypeTable()->row(row, et);
			return true;
		}

		return false;
	}
};

class Hidden : public BoolTableColumn
{
public:
	Hidden(DataModel* dataModel) : BoolTableColumn("Hidden", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return dataModel->eventTypeTable()->row(row).hidden;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = dataModel->eventTypeTable()->row(row);
			et.hidden = value.toBool();
			dataModel->eventTypeTable()->row(row, et);
			return true;
		}

		return false;
	}
};

} // namespace

EventTypeTableModel::EventTypeTableModel(DataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Id(file->getDataModel()));
	columns.push_back(new Name(file->getDataModel()));
	columns.push_back(new Opacity(file->getDataModel()));
	columns.push_back(new Color(file->getDataModel()));
	columns.push_back(new Hidden(file->getDataModel()));

	auto vitness = VitnessEventTypeTable::vitness(file->getDataModel()->eventTypeTable());
	connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(emitDataChanged(int, int)));
	connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
}

int EventTypeTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return file->getDataModel()->eventTypeTable()->rowCount();
}

void EventTypeTableModel::insertRowBack()
{
	file->getDataModel()->eventTypeTable()->insertRows(file->getDataModel()->eventTypeTable()->rowCount());
}

void EventTypeTableModel::removeRowsFromDataModel(int row, int count)
{
	// Update the types of events to point to correct event types after the rows are removed.
	for (int i = 0; i < file->getDataModel()->montageTable()->rowCount(); ++i)
	{
		AbstractEventTable* eventTable = file->getDataModel()->montageTable()->eventTable(i);

		for (int j = 0; j < eventTable->rowCount(); ++j)
		{
			Event e = eventTable->row(j);

			if (row + count - 1 < e.type)
				e.type -= count;
			else if (row <= e.type && e.type <= row + count - 1)
				e.type = -1;

			eventTable->row(j, e);
		}
	}

	file->getDataModel()->eventTypeTable()->removeRows(row, count);
}
