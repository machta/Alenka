#include "tablemodel.h"

#include "../DataModel/infotable.h"
#include "../DataModel/opendatafile.h"
#include "dummywidget.h"

#include <QAction>
#include <QCollator>
#include <QColorDialog>
#include <QLineEdit>

#include <cassert>

using namespace std;

namespace {

class Delegate : public QStyledItemDelegate {
  vector<unique_ptr<TableColumn>> *columns;

public:
  explicit Delegate(vector<unique_ptr<TableColumn>> *columns,
                    QObject *parent = nullptr)
      : QStyledItemDelegate(parent), columns(columns) {}

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override {
    QWidget *widget;
    if ((*columns)[index.column()]->createEditor(this, parent, option, index,
                                                 &widget))
      return widget;
    else
      return QStyledItemDelegate::createEditor(parent, option, index);
  }
  void setEditorData(QWidget *editor, const QModelIndex &index) const override {
    if (!(*columns)[index.column()]->setEditorData(this, editor, index))
      QStyledItemDelegate::setEditorData(editor, index);
  }
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override {
    if (!(*columns)[index.column()]->setModelData(this, editor, model, index))
      QStyledItemDelegate::setModelData(editor, model, index);
  }
};

} // namespace

bool BoolTableColumn::createEditor(const QStyledItemDelegate *delegate,
                                   QWidget *parent,
                                   const QStyleOptionViewItem & /*option*/,
                                   const QModelIndex & /*index*/,
                                   QWidget **widget) const {
  function<void(void)> fun = [widget, delegate]() {
    emit const_cast<QStyledItemDelegate *>(delegate)->commitData(*widget);
    emit const_cast<QStyledItemDelegate *>(delegate)->closeEditor(*widget);
  };

  *widget = new DummyWidget(fun, parent);
  return true;
}

bool BoolTableColumn::setModelData(const QStyledItemDelegate * /*delegate*/,
                                   QWidget * /*editor*/,
                                   QAbstractItemModel *model,
                                   const QModelIndex &index) const {
  // Just flip the value;
  model->setData(index, !index.data(Qt::EditRole).toBool());

  return true;
}

bool ColorTableColumn::createEditor(const QStyledItemDelegate *delegate,
                                    QWidget *parent,
                                    const QStyleOptionViewItem & /*option*/,
                                    const QModelIndex & /*index*/,
                                    QWidget **widget) const {
  auto lineEdit = new QLineEdit(parent);
  QAction *action = lineEdit->addAction(QIcon(":/icons/edit.png"),
                                        QLineEdit::TrailingPosition);

  lineEdit->connect(action, &QAction::triggered, [lineEdit, delegate]() {
    QColor color;
    color.setNamedColor(lineEdit->text());

    color = QColorDialog::getColor(color, lineEdit);

    if (color.isValid()) {
      lineEdit->setText(color.name());

      emit const_cast<QStyledItemDelegate *>(delegate)->commitData(lineEdit);
      emit const_cast<QStyledItemDelegate *>(delegate)->closeEditor(lineEdit);
    }
  });

  *widget = lineEdit;
  return true;
}

TableModel::TableModel(OpenDataFile *file, QObject *parent)
    : QAbstractTableModel(parent), file(file),
      delegate(new Delegate(&columns)) {}

bool TableModel::removeRows(int row, int count,
                            const QModelIndex & /*parent*/) {
  if (count > 0 && areAllRowsDeletable(row, count)) {
    int rowLast = row + count - 1;
    assert(rowLast < rowCount());

    beginRemoveRows(QModelIndex(), row, rowLast);
    removeRowsFromDataModel(row, count);
    endRemoveRows();

    return true;
  }

  return false;
}

void TableModel::insertDataModelRows(int row, int count) {
  if (0 < count) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
  }
}

void TableModel::removeDataModelRows(int row, int count) {
  if (0 < count) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
  }
}
