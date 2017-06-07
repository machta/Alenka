#ifndef EVENTTYPETABLEMODEL_H
#define EVENTTYPETABLEMODEL_H

#include "tablemodel.h"

class EventTypeTableModel : public TableModel {
  Q_OBJECT

public:
  explicit EventTypeTableModel(OpenDataFile *file, QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
  void removeRowsFromDataModel(int row, int count) override;
};

#endif // EVENTTYPETABLEMODEL_H
