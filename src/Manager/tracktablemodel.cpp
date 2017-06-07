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

using namespace std;
using namespace AlenkaFile;

namespace {

const AbstractTrackTable *currentTrackTable(OpenDataFile *file) {
  return file->dataModel->montageTable()->trackTable(
      OpenDataFile::infoTable.getSelectedMontage());
}

class Id : public TableColumn {
public:
  Id(OpenDataFile *file) : TableColumn("ID", file) {}

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return row;

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    (void)row;
    (void)value;
    (void)role;
    return false;
  }

  Qt::ItemFlags flags() const override { return Qt::NoItemFlags; }
};

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
public:
  Code(OpenDataFile *file) : TableColumn("Code", file) {
    validator = new TrackCodeValidator();
  }
  ~Code() override { delete validator; }

  QVariant data(int row, int role) const override {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
      return QString::fromStdString(currentTrackTable(file)->row(row).code);

    return QVariant();
  }

  bool setData(int row, const QVariant &value, int role) override {
    if (role == Qt::EditRole) {
      Track t = currentTrackTable(file)->row(row);
      QString qc = value.toString();
      string c = qc.toStdString();

      if (t.code != c && validator->validate(qc)) {
        t.code = c;
        file->undoFactory->changeTrack(
            OpenDataFile::infoTable.getSelectedMontage(), row, t,
            "change Code");
        return true;
      }
    }

    return false;
  }

  bool createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                    const QModelIndex &index, QWidget **widget) const override {
    (void)option;
    (void)index;

    auto lineEdit = new QLineEdit(parent);
    QAction *action = lineEdit->addAction(QIcon(":/icons/edit.png"),
                                          QLineEdit::TrailingPosition);

    lineEdit->connect(action, &QAction::triggered, [lineEdit]() {
      CodeEditDialog dialog(lineEdit);
      dialog.setText(lineEdit->text());
      int result = dialog.exec();

      if (result == QDialog::Accepted) {
        lineEdit->setText(dialog.getText());

        // TODO: Decide whether to turn this back on, or leave it out.
        // emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
        // emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
      }
    });

    *widget = lineEdit;
    return true;
  }

  bool setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override {
    (void)model;
    (void)index;

    QLineEdit *lineEdit = reinterpret_cast<QLineEdit *>(editor);
    QString message;

    if (!validator->validate(lineEdit->text(), &message)) {
      CodeEditDialog::errorMessageDialog(message, editor);
      return true;
    }

    return false;
  }

private:
  TrackCodeValidator *validator;
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
      DataModel::color2array(value.value<QColor>(), t.color);
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

      QLocale locale;
      bool ok;
      double tmp = value.toDouble(&ok);
      if (ok) {
        if (t.amplitude == tmp)
          return false;
        t.amplitude = tmp;
      } else {
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

  bool createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                    const QModelIndex &index, QWidget **widget) const override {
    (void)option;
    (void)index;

    auto spinBox = new QDoubleSpinBox(parent);
    spinBox->setDecimals(10);
    spinBox->setRange(std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max());

    *widget = spinBox;
    return true;
  }
};

class Hidden : public BoolTableColumn {
public:
  Hidden(OpenDataFile *file) : BoolTableColumn("Hidden", file) {}

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

} // namespace

TrackTableModel::TrackTableModel(OpenDataFile *file, QObject *parent)
    : TableModel(file, parent) {
  columns.push_back(new Id(file));
  columns.push_back(new Label(file));
  columns.push_back(new Code(file));
  columns.push_back(new Color(file));
  columns.push_back(new Amplitude(file));
  columns.push_back(new Hidden(file));

  connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this,
          SLOT(setSelectedMontage(int)));
  setSelectedMontage(OpenDataFile::infoTable.getSelectedMontage());
}

int TrackTableModel::rowCount(const QModelIndex &parent) const {
  (void)parent;

  if (0 < file->dataModel->montageTable()->rowCount())
    return currentTrackTable(file)->rowCount();
  return 0;
}

void TrackTableModel::removeRowsFromDataModel(int row, int count) {
  file->undoFactory->beginMacro("remove EventTable rows");
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
  file->undoFactory->endMacro();
}

void TrackTableModel::setSelectedMontage(int i) {
  beginResetModel();

  for (auto e : trackTableConnections)
    disconnect(e);
  trackTableConnections.clear();

  if (0 < file->dataModel->montageTable()->rowCount()) {
    auto vitness = VitnessTrackTable::vitness(
        file->dataModel->montageTable()->trackTable(i));

    auto c =
        connect(vitness, &DataModelVitness::valueChanged,
                [this](int row, int col) { emitDataChanged(row, col + 1); });
    trackTableConnections.push_back(c);

    c = connect(vitness, SIGNAL(rowsInserted(int, int)), this,
                SLOT(insertDataModelRows(int, int)));
    trackTableConnections.push_back(c);

    c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
                SLOT(removeDataModelRows(int, int)));
    trackTableConnections.push_back(c);
  }

  endResetModel();
}
