#include "montagetablemodel.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Name : public StringTableColumn
{
public:
	Name(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Name", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.montageTable->row(row).name);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = dataModel.montageTable->row(row);
			m.name = value.toString().toStdString();
			dataModel.montageTable->row(row, m);
			return true;
		}

		return false;
	}
};

class Save : public BoolTableColumn
{
public:
	Save(InfoTable* infoTable, DataModel dataModel) : BoolTableColumn("Save", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.montageTable->row(row).save;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Montage m = dataModel.montageTable->row(row);
			m.save = value.toBool();
			dataModel.montageTable->row(row, m);
			return true;
		}

		return false;
	}
};

} // namespace

MontageTableModel::MontageTableModel(InfoTable* infoTable, DataModel dataModel, QObject* parent) : TableModel(infoTable, dataModel, parent)
{
	columns.push_back(new Name(infoTable, dataModel));
	columns.push_back(new Save(infoTable, dataModel));
}
