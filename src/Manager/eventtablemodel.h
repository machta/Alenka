#ifndef EVENTTABLEMODEL_H
#define EVENTTABLEMODEL_H

#include "tablemodel.h"

#include <vector>

class EventTableModel : public TableModel {
  Q_OBJECT

  std::vector<QMetaObject::Connection> connections;

public:
  explicit EventTableModel(OpenDataFile *file, QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
  void removeRowsFromDataModel(int row, int count) override;

private slots:
  void selectMontage(int montageIndex);
  void beginEndReset() {
    beginResetModel();
    endResetModel();
  }
};

#endif // EVENTTABLEMODEL_H
