#include "montagetablemodel.h"

#include "../DataModel/vitnessdatamodel.h"
#include "../DataModel/undocommandfactory.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Name : public TableColumn
{
public:
	Name(OpenDataFile* file) : TableColumn("Name", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(file->dataModel->montageTable()->row(row).name);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = file->dataModel->montageTable()->row(row);
			m.name = value.toString().toStdString();
			file->undoFactory->changeMontage(row, m, "change Name");
			return true;
		}

		return false;
	}
};

class Save : public BoolTableColumn
{
public:
	Save(OpenDataFile* file) : BoolTableColumn("Save", file) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return file->dataModel->montageTable()->row(row).save;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = file->dataModel->montageTable()->row(row);
			m.save = value.toBool();
			file->undoFactory->changeMontage(row, m, "change Save");
			return true;
		}

		return false;
	}
};

} // namespace

MontageTableModel::MontageTableModel(OpenDataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Name(file));
	columns.push_back(new Save(file));

	auto vitness = VitnessMontageTable::vitness(file->dataModel->montageTable());
	connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(emitDataChanged(int, int)));
	connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
	connect(vitness, SIGNAL(rowsRemoved(int, int)), this, SLOT(removeDataModelRows(int, int)));
}

int MontageTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return file->dataModel->montageTable()->rowCount();
}

void MontageTableModel::removeRowsFromDataModel(int row, int count)
{
	file->undoFactory->beginMacro("remove Montage rows");
	file->undoFactory->removeMontage(row, count);
	file->undoFactory->endMacro();
}
