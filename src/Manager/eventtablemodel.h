#ifndef EVENTTABLEMODEL_H
#define EVENTTABLEMODEL_H

#include "tablemodel.h"

#include <vector>

class EventTableModel : public TableModel
{
	Q_OBJECT

public:
	explicit EventTableModel(OpenDataFile* file, QObject* parent = nullptr);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

protected:
	virtual void removeRowsFromDataModel(int row, int count) override;

private slots:
	void setSelectedMontage(int i);
	void beginEndReset()
	{
		beginResetModel();
		endResetModel();
	}

private:
	std::vector<QMetaObject::Connection> montageTableConnections;
};

#endif // EVENTTABLEMODEL_H
