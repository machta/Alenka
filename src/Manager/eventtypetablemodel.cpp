#include "eventtypetablemodel.h"

#include "../DataModel/opendatafile.h"
#include "../signalfilebrowserwindow.h"
#include "../DataModel/vitnessdatamodel.h"
#include "../DataModel/undocommandfactory.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Id : public TableColumn
{
public:
	Id(OpenDataFile* file) : TableColumn("ID", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return file->dataModel->eventTypeTable()->row(row).id;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = file->dataModel->eventTypeTable()->row(row);
			et.id = value.toInt();
			file->undoFactory->changeEventType(row, et, "change ID");
			return true;
		}

		return false;
	}
};

class Name : public TableColumn
{
public:
	Name(OpenDataFile* file) : TableColumn("Name", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(file->dataModel->eventTypeTable()->row(row).name);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = file->dataModel->eventTypeTable()->row(row);
			et.name = value.toString().toStdString();
			file->undoFactory->changeEventType(row, et, "change Name");
			return true;
		}

		return false;
	}
};

class Opacity : public TableColumn
{
public:
	Opacity(OpenDataFile* file) : TableColumn("Opacity", file) {}

	virtual QVariant data(int row, int role) const override
	{
		double opacity = file->dataModel->eventTypeTable()->row(row).opacity*100;

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
			EventType et = file->dataModel->eventTypeTable()->row(row);

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
			file->undoFactory->changeEventType(row, et, "change Opacity");
			return true;
		}

		return false;
	}
};

class Color : public ColorTableColumn
{
public:
	Color(OpenDataFile* file) : ColorTableColumn("Color", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole)
		{
			auto colorArray = file->dataModel->eventTypeTable()->row(row).color;
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
			EventType et = file->dataModel->eventTypeTable()->row(row);
			DataModel::color2array(value.value<QColor>(), et.color);
			file->undoFactory->changeEventType(row, et, "change Color");
			return true;
		}

		return false;
	}
};

class Hidden : public BoolTableColumn
{
public:
	Hidden(OpenDataFile* file) : BoolTableColumn("Hidden", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return file->dataModel->eventTypeTable()->row(row).hidden;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			EventType et = file->dataModel->eventTypeTable()->row(row);
			et.hidden = value.toBool();
			file->undoFactory->changeEventType(row, et, "change Hidden");
			return true;
		}

		return false;
	}
};

} // namespace

EventTypeTableModel::EventTypeTableModel(OpenDataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Id(file));
	columns.push_back(new Name(file));
	columns.push_back(new Opacity(file));
	columns.push_back(new Color(file));
	columns.push_back(new Hidden(file));

	auto vitness = VitnessEventTypeTable::vitness(file->dataModel->eventTypeTable());
	connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(emitDataChanged(int, int)));
	connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
	connect(vitness, SIGNAL(rowsRemoved(int, int)), this, SLOT(removeDataModelRows(int, int)));
}

int EventTypeTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return file->dataModel->eventTypeTable()->rowCount();
}

void EventTypeTableModel::removeRowsFromDataModel(int row, int count)
{
	file->undoFactory->beginMacro("remove EventTypeTable rows");

	for (int i = 0; i < file->dataModel->montageTable()->rowCount(); ++i)
	{
		// Update the types of events to point to correct values after the rows are removed.
		const AbstractEventTable* eventTable = file->dataModel->montageTable()->eventTable(i);

		for (int j = 0; j < eventTable->rowCount(); ++j)
		{
			Event e = eventTable->row(j);

			if (row + count - 1 < e.type)
				e.type -= count;
			else if (row <= e.type && e.type <= row + count - 1)
				e.type = -1;

			file->undoFactory->changeEvent(i, j, e);
		}
	}

	file->undoFactory->removeEventType(row, count);
	file->undoFactory->endMacro();
}
