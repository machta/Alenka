#include "sortproxymodel.h"

#include <QCollator>

#include <cassert>

bool SortProxyModel::lessThan(const QModelIndex &left,
                              const QModelIndex &right) const {
  QVariant leftData = sourceModel()->data(left);
  QVariant rightData = sourceModel()->data(right);

  if (leftData.type() == QVariant::String) {
    assert(rightData.type() == QVariant::String);

    QCollator collator;
    collator.setNumericMode(true);

    return collator.compare(leftData.toString(), rightData.toString()) < 0;
  } else {
    return QSortFilterProxyModel::lessThan(left, right);
  }
}
