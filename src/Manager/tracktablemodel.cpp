#include "tracktablemodel.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/trackcodevalidator.h"
#include "../DataModel/undocommandfactory.h"
#include "../DataModel/vitnessdatamodel.h"
#include "codeeditdialog.h"

#include <QAction>
#include <QColor>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLocale>

#include <memory>

using namespace std;
using namespace AlenkaFile;

namespace {

const AbstractTrackTable *currentTrackTable(OpenDataFile *file) {
  return file->dataModel->montageTable()->trackTable(
      OpenDataFile::infoTable.getSelectedMontage());
}

class Label : public TableColumn {
public:
  Label(OpenDataFile *file) : TableColumn("Label", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return QString::fromStdString(currentTrackTable(file)->row(row).label);

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      t.label = value.toString().toStdString();
      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t, "change Label");
      return true;
    }

    return false;
  }
};

class Code : public TableColumn {
  unique_ptr<TrackCodeValidator> validator;

public:
  Code(OpenDataFile *file)
      : TableColumn("Code", file), validator(new TrackCodeValidator()) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return QString::fromStdString(currentTrackTable(file)->row(row).code);

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    const int selected = OpenDataFile::infoTable.getSelectedMontage();

    // Code for montage with index 0 shouldn't be edited.
    if (role == Qt::EditRole && 0 != selected) {
      Track t = currentTrackTable(file)->row(row);
      const QString qc = value.toString();
      const string c = qc.toStdString();

      if (t.code != c &&
          validator->validate(
              qc, OpenDataFile::infoTable.getGlobalMontageHeader())) {
        t.code = c;
        file->undoFactory->changeTrack(
            OpenDataFile::infoTable.getSelectedMontage(), row, t,
            "change Code");
        return true;
      }
    }

    return false;
  }

  Qt::ItemFlags flags(int row) const override {
    if (file->infoTable.getSelectedMontage() == 0)
      return Qt::NoItemFlags;

    return TableColumn::flags(row);
  }

  bool createEditor(const QStyledItemDelegate *delegate, QWidget *parent,
                    const QStyleOptionViewItem & /*option*/,
                    const QModelIndex & /*index*/,
                    QWidget **widget) const override {
    auto lineEdit = new QLineEdit(parent);
    QAction *action = lineEdit->addAction(QIcon(":/icons/edit.png"),
                                          QLineEdit::TrailingPosition);

    lineEdit->connect(action, &QAction::triggered, [lineEdit, delegate]() {
      CodeEditDialog dialog(lineEdit);
      dialog.setText(lineEdit->text());
      int result = dialog.exec();

      if (result == QDialog::Accepted) {
        lineEdit->setText(dialog.getText());

        emit const_cast<QStyledItemDelegate *>(delegate)->commitData(lineEdit);
        emit const_cast<QStyledItemDelegate *>(delegate)->closeEditor(lineEdit);
      }
    });

    *widget = lineEdit;
    return true;
  }

  bool setModelData(const QStyledItemDelegate * /*delegate*/, QWidget *editor,
                    QAbstractItemModel * /*model*/,
                    const QModelIndex & /*index*/) const override {
    QLineEdit *lineEdit = reinterpret_cast<QLineEdit *>(editor);
    QString message;

    if (!validator->validate(lineEdit->text(),
                             OpenDataFile::infoTable.getGlobalMontageHeader(),
                             &message)) {
      CodeEditDialog::errorMessageDialog(message, editor);
      return true;
    }

    return false;
  }
};

class Color : public ColorTableColumn {
public:
  Color(OpenDataFile *file) : ColorTableColumn("Color", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole ||
        role == Qt::DecorationRole) {
      auto colorArray = currentTrackTable(file)->row(row).color;
      QColor color;
      color.setRgb(colorArray[0], colorArray[1], colorArray[2]);
      return color;
    }

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      t.color = DataModel::color2colorArray(value.value<QColor>());
      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t, "change Color");
      return true;
    }

    return false;
  }
};

class Amplitude : public TableColumn {
public:
  Amplitude(OpenDataFile *file) : TableColumn("Amplitude", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return currentTrackTable(file)->row(row).amplitude;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      bool ok;
      double tmp = value.toDouble(&ok);

      if (ok) {
        if (t.amplitude == tmp)
          return false;
        t.amplitude = tmp;
      } else {
        QLocale locale;
        tmp = locale.toDouble(value.toString());
        if (t.amplitude == tmp)
          return false;
        t.amplitude = tmp;
      }

      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t,
          "change Amplitude");
      return true;
    }

    return false;
  }

  bool createEditor(const QStyledItemDelegate * /*delegate*/, QWidget *parent,
                    const QStyleOptionViewItem & /*option*/,
                    const QModelIndex & /*index*/,
                    QWidget **widget) const override {
    auto spinBox = new QDoubleSpinBox(parent);
    spinBox->setDecimals(10);
    spinBox->setRange(std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max());

    *widget = spinBox;
    return true;
  }
};

class Hidden : public TableColumn {
public:
  Hidden(OpenDataFile *file) : TableColumn("Hidden", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return currentTrackTable(file)->row(row).hidden;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      t.hidden = value.toBool();
      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t,
          "change Hidden");
      return true;
    }

    return false;
  }
};

class X : public TableColumn {
public:
  X(OpenDataFile *file) : TableColumn("X", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return currentTrackTable(file)->row(row).x;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      bool ok;
      float tmp = value.toFloat(&ok);

      if (ok) {
        if (t.x == tmp)
          return false;
        t.x = tmp;
      } else {
        QLocale locale;
        tmp = locale.toFloat(value.toString());
        if (t.x == tmp)
          return false;
        t.x = tmp;
      }

      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t, "change X");
      return true;
    }

    return false;
  }
};

class Y : public TableColumn {
public:
  Y(OpenDataFile *file) : TableColumn("Y", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return currentTrackTable(file)->row(row).y;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      bool ok;
      float tmp = value.toFloat(&ok);

      if (ok) {
        if (t.y == tmp)
          return false;
        t.y = tmp;
      } else {
        QLocale locale;
        tmp = locale.toFloat(value.toString());
        if (t.y == tmp)
          return false;
        t.y = tmp;
      }

      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t, "change Y");
      return true;
    }

    return false;
  }
};

class Z : public TableColumn {
public:
  Z(OpenDataFile *file) : TableColumn("Z", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return currentTrackTable(file)->row(row).z;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      bool ok;
      float tmp = value.toFloat(&ok);

      if (ok) {
        if (t.z == tmp)
          return false;
        t.z = tmp;
      } else {
        QLocale locale;
        tmp = locale.toFloat(value.toString());
        if (t.z == tmp)
          return false;
        t.z = tmp;
      }

      file->undoFactory->changeTrack(
          OpenDataFile::infoTable.getSelectedMontage(), row, t, "change Z");
      return true;
    }

    return false;
  }
};

} // namespace

TrackTableModel::TrackTableModel(OpenDataFile *file, QObject *parent)
    : TableModel(file, parent) {
  columns.push_back(make_unique<ConstId>(file));
  columns.push_back(make_unique<Label>(file));
  columns.push_back(make_unique<Code>(file));
  columns.push_back(make_unique<Color>(file));
  columns.push_back(make_unique<Amplitude>(file));
  columns.push_back(make_unique<Hidden>(file));
  columns.push_back(make_unique<X>(file));
  columns.push_back(make_unique<Y>(file));
  columns.push_back(make_unique<Z>(file));

  connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this,
          SLOT(selectMontage(int)));
  selectMontage(OpenDataFile::infoTable.getSelectedMontage());
}

int TrackTableModel::rowCount(const QModelIndex & /*parent*/) const {
  if (0 < file->dataModel->montageTable()->rowCount())
    return currentTrackTable(file)->rowCount();
  return 0;
}

void TrackTableModel::removeRowsFromDataModel(int row, int count) {
  for (int i = 0; i < file->dataModel->montageTable()->rowCount(); ++i) {
    // Update the channels of events to point to correct tracks after the rows
    // are removed.
    const AbstractEventTable *eventTable =
        file->dataModel->montageTable()->eventTable(i);

    for (int j = 0; j < eventTable->rowCount(); ++j) {
      Event e = eventTable->row(j);

      if (row + count - 1 < e.channel)
        e.channel -= count;
      else if (row <= e.channel && e.channel <= row + count - 1)
        e.channel = -2;

      file->undoFactory->changeEvent(i, j, e);
    }
  }

  file->undoFactory->removeTrack(OpenDataFile::infoTable.getSelectedMontage(),
                                 row, count);
}

bool TrackTableModel::areAllRowsDeletable(int /*row*/, int /*count*/) {
  // Montage with index 0 shouldn't be edited.
  return 0 != file->infoTable.getSelectedMontage();
}

void TrackTableModel::selectMontage(const int montageIndex) {
  beginResetModel();

  for (auto e : connections)
    disconnect(e);
  connections.clear();

  if (0 < file->dataModel->montageTable()->rowCount()) {
    auto vitness = VitnessTrackTable::vitness(
        file->dataModel->montageTable()->trackTable(montageIndex));

    auto c =
        connect(vitness, &DataModelVitness::valueChanged,
                [this](int row, int col) { emitDataChanged(row, col + 1); });
    connections.push_back(c);

    c = connect(vitness, SIGNAL(rowsInserted(int, int)), this,
                SLOT(insertDataModelRows(int, int)));
    connections.push_back(c);

    c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
                SLOT(removeDataModelRows(int, int)));
    connections.push_back(c);
  }

  endResetModel();
}
