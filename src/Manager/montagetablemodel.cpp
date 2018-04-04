#include "montagetablemodel.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"
#include "../DataModel/vitnessdatamodel.h"

#include <QColor>
#include <QLocale>

using namespace std;
using namespace AlenkaFile;

namespace {

class Name : public TableColumn {
public:
  Name(OpenDataFile *file) : TableColumn("Name", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return QString::fromStdString(
          file->dataModel->montageTable()->row(row).name);

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Montage m = file->dataModel->montageTable()->row(row);
      m.name = value.toString().toStdString();
      file->undoFactory->changeMontage(row, m, "change Name");
      return true;
    }

    return false;
  }

  Qt::ItemFlags flags(int row) const override {
    if (row == 0)
      return Qt::NoItemFlags;

    return TableColumn::flags(row);
  }
};

class Save : public TableColumn {
public:
  Save(OpenDataFile *file) : TableColumn("Save", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return file->dataModel->montageTable()->row(row).save;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Montage m = file->dataModel->montageTable()->row(row);
      m.save = value.toBool();
      file->undoFactory->changeMontage(row, m, "change Save");
      return true;
    }

    return false;
  }
};

} // namespace

MontageTableModel::MontageTableModel(OpenDataFile *file, QObject *parent)
    : TableModel(file, parent) {
  columns.push_back(make_unique<Name>(file));
  columns.push_back(make_unique<Save>(file));

  auto vitness = VitnessMontageTable::vitness(file->dataModel->montageTable());
  connect(vitness, SIGNAL(valueChanged(int, int)), this,
          SLOT(emitDataChanged(int, int)));
  connect(vitness, SIGNAL(rowsInserted(int, int)), this,
          SLOT(insertDataModelRows(int, int)));
  connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
          SLOT(removeDataModelRows(int, int)));
}

int MontageTableModel::rowCount(const QModelIndex & /*parent*/) const {
  return file->dataModel->montageTable()->rowCount();
}

void MontageTableModel::removeRowsFromDataModel(int row, int count) {
  file->undoFactory->removeMontage(row, count);
}
