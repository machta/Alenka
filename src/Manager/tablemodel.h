#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QStyledItemDelegate>

#include <functional>
#include <memory>
#include <string>
#include <vector>

const std::string NO_TYPE_STRING = "<No Type>";
const std::string NO_CHANNEL_STRING = "<No Channel>";
const std::string ALL_CHANNEL_STRING = "<All>";

class OpenDataFile;

class TableColumn {
public:
  TableColumn(QString header, OpenDataFile *file)
      : header(std::move(header)), file(file) {}
  virtual ~TableColumn() = default;

  virtual QVariant headerData() const { return header; }

  virtual QVariant data(int row, int role = Qt::DisplayRole) const = 0;
  virtual bool setData(int row, const QVariant &value,
                       int role = Qt::EditRole) = 0;
  virtual Qt::ItemFlags flags(int /*row*/) const { return Qt::ItemIsEditable; }

  virtual bool createEditor(const QStyledItemDelegate * /*delegate*/,
                            QWidget * /*parent*/,
                            const QStyleOptionViewItem & /*option*/,
                            const QModelIndex & /*index*/,
                            QWidget ** /*widget*/) const {
    return false;
  }
  virtual bool setEditorData(const QStyledItemDelegate * /*delegate*/,
                             QWidget * /*editor*/,
                             const QModelIndex & /*index*/) const {
    return false;
  }
  virtual bool setModelData(const QStyledItemDelegate * /*delegate*/,
                            QWidget * /*editor*/,
                            QAbstractItemModel * /*model*/,
                            const QModelIndex & /*index*/) const {
    return false;
  }

protected:
  QString header;
  OpenDataFile *file;
};

class ConstId : public TableColumn {
public:
  ConstId(OpenDataFile *file) : TableColumn("ID", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return row;

    return QVariant();
  }

  bool setData(int /*row*/, const QVariant & /*value*/, int /*role*/) override {
    return false;
  }

  Qt::ItemFlags flags(int /*row*/) const override { return Qt::NoItemFlags; }
};

class BoolTableColumn : public TableColumn {
public:
  BoolTableColumn(const QString &header, OpenDataFile *file)
      : TableColumn(header, file) {}

  bool createEditor(const QStyledItemDelegate *delegate, QWidget *parent,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index, QWidget **widget) const override;
  bool setModelData(const QStyledItemDelegate *delegate, QWidget *editor,
                    QAbstractItemModel *model,
                    const QModelIndex &index) const override;
};

class ColorTableColumn : public TableColumn {
public:
  ColorTableColumn(const QString &header, OpenDataFile *file)
      : TableColumn(header, file) {}

  bool createEditor(const QStyledItemDelegate *delegate, QWidget *parent,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index, QWidget **widget) const override;
};

class TableModel : public QAbstractTableModel {
  Q_OBJECT

protected:
  OpenDataFile *file;
  std::vector<std::unique_ptr<TableColumn>> columns;

private:
  QStyledItemDelegate *delegate;

public:
  explicit TableModel(OpenDataFile *file, QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override = 0;
  int columnCount(
      const QModelIndex & /*parent*/ = QModelIndex()) const override {
    return static_cast<int>(columns.size());
  }
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal &&
        0 <= section && section < columnCount())
      return columns[section]->headerData();

    return QVariant();
  }
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override {
    if (index.isValid() && index.row() < rowCount() &&
        index.column() < columnCount())
      return columns[index.column()]->data(index.row(), role);

    return QVariant();
  }
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override {
    if (index.isValid() && index.row() < rowCount() &&
        index.column() < columnCount())
      return columns[index.column()]->setData(index.row(), value, role);

    return false;
  }
  Qt::ItemFlags flags(const QModelIndex &index) const override {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) |
           columns[index.column()]->flags(index.row());
  }
  bool removeRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;

  QStyledItemDelegate *getDelegate() { return delegate; }

public slots:
  void emitColumnChanged(int column) {
    int c = static_cast<int>(column);
    emit dataChanged(index(0, c), index(rowCount() - 1, c));
  }

protected:
  virtual void removeRowsFromDataModel(int row, int count) = 0;
  virtual bool areAllRowsDeletable(int /*row*/, int /*count*/) { return true; }

protected slots:
  void emitDataChanged(int row, int col) {
    QModelIndex i = index(row, col);
    emit dataChanged(i, i);
  }

  void insertDataModelRows(int row, int count);
  void removeDataModelRows(int row, int count);
};

#endif // TABLEMODEL_H
