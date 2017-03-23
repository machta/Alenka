#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include "tablemodel.h"

#include <vector>

class TrackTableModel : public TableModel
{
	Q_OBJECT

public:
	explicit TrackTableModel(OpenDataFile* file, QObject* parent = nullptr);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

protected:
	virtual void removeRowsFromDataModel(int row, int count) override;

private slots:
	void setSelectedMontage(int i);

private:
	std::vector<QMetaObject::Connection> trackTableConnections;
};

#endif // TRACKTABLEMODEL_H
