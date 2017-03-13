#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include "tablemodel.h"

class TrackTableModel : public TableModel
{
	Q_OBJECT

public:
	TrackTableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent = nullptr);

protected:
	virtual void insertRowBack() override
	{
		int rc = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->rowCount();
		dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->insertRows(rc);
	}
};


#endif // TRACKTABLEMODEL_H
