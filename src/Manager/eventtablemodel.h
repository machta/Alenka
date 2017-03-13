#ifndef EVENTTABLEMODEL_H
#define EVENTTABLEMODEL_H

#include "tablemodel.h"

class EventTableModel : public TableModel
{
	Q_OBJECT

public:
	EventTableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent = nullptr);

protected:
	virtual void insertRowBack() override
	{
		int rc = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->rowCount();
		dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->insertRows(rc);
	}
};

#endif // EVENTTABLEMODEL_H
