#include "montagetablemodel.h"

#include "../DataModel/vitnessdatamodel.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Name : public TableColumn
{
public:
	Name(DataModel* dataModel) : TableColumn("Name", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(dataModel->montageTable()->row(row).name);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = dataModel->montageTable()->row(row);
			m.name = value.toString().toStdString();
			dataModel->montageTable()->row(row, m);
			return true;
		}

		return false;
	}
};

class Save : public BoolTableColumn
{
public:
	Save(DataModel* dataModel) : BoolTableColumn("Save", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return dataModel->montageTable()->row(row).save;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = dataModel->montageTable()->row(row);
			m.save = value.toBool();
			dataModel->montageTable()->row(row, m);
			return true;
		}

		return false;
	}
};

} // namespace

MontageTableModel::MontageTableModel(OpenDataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Name(file->dataModel));
	columns.push_back(new Save(file->dataModel));

	auto vitness = VitnessMontageTable::vitness(file->dataModel->montageTable());
	connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(emitDataChanged(int, int)));
	connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
}

int MontageTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return file->dataModel->montageTable()->rowCount();
}

void MontageTableModel::insertRowBack()
{
	file->dataModel->montageTable()->insertRows(file->dataModel->montageTable()->rowCount());
}

void MontageTableModel::removeRowsFromDataModel(int row, int count)
{
	file->dataModel->montageTable()->removeRows(row, count);
}
