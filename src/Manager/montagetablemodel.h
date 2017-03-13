#ifndef MONTAGETABLEMODEL_H
#define MONTAGETABLEMODEL_H

#include "tablemodel.h"

class MontageTableModel : public TableModel
{
	Q_OBJECT

public:
	MontageTableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent = nullptr);

protected:
	virtual void insertRowBack() override
	{
		dataModel.montageTable->insertRows(dataModel.montageTable->rowCount());
	}
};

#endif // MONTAGETABLEMODEL_H
