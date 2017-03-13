#ifndef EVENTTYPETABLEMODEL_H
#define EVENTTYPETABLEMODEL_H

#include "tablemodel.h"

class EventTypeTableModel : public TableModel
{
	Q_OBJECT

public:
	EventTypeTableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent = nullptr);

protected:
	virtual void insertRowBack() override
	{
		dataModel.eventTypeTable->insertRows(dataModel.eventTypeTable->rowCount());
	}
};


#endif // EVENTTYPETABLEMODEL_H
