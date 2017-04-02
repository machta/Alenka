#ifndef MONTAGETABLEMODEL_H
#define MONTAGETABLEMODEL_H

#include "tablemodel.h"

class MontageTableModel : public TableModel
{
	Q_OBJECT

public:
	explicit MontageTableModel(OpenDataFile* file, QObject* parent = nullptr);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

protected:
	virtual void removeRowsFromDataModel(int row, int count) override;
};

#endif // MONTAGETABLEMODEL_H
