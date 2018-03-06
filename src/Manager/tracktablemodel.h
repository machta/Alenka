#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include "tablemodel.h"

#include <vector>

class TrackTableModel : public TableModel {
  Q_OBJECT

  std::vector<QMetaObject::Connection> connections;

public:
  explicit TrackTableModel(OpenDataFile *file, QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
  void removeRowsFromDataModel(int row, int count) override;
  bool areAllRowsDeletable(int row, int count) override;

private slots:
  void selectMontage(int montageIndex);
};

#endif // TRACKTABLEMODEL_H
