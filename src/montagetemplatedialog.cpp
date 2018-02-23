#include "montagetemplatedialog.h"

#include "../Alenka-File/include/AlenkaFile/datafile.h"
#include "DataModel/opendatafile.h"
#include "DataModel/undocommandfactory.h"
#include "error.h"
#include "myapplication.h"

#include <pugixml.hpp>

#include <QAction>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>
#include <cassert>

using namespace std;
using namespace pugi;
using namespace AlenkaFile;

namespace {

auto readRows(const QFileInfo &fi) {
  vector<array<QString, 2>> rows;
  string filePath = fi.filePath().toStdString();
  xml_document xmlFile;
  xml_parse_result res = xmlFile.load_file(filePath.c_str());

  if (res) {
    xml_node document = xmlFile.child("document");
    xml_node row = document.child("row");

    while (row) {
      rows.emplace_back();

      rows.back()[0] = QString(row.attribute("label").as_string()).trimmed();
      rows.back()[1] = QString(row.child("code").text().as_string()).trimmed();

      row = row.next_sibling("row");
    }
  } else {
    cerr << "Error while opening file '" << filePath << "'" << endl;
  }
  assert(res && "Error while opening file");

  return rows;
}

QString indentString(const QString &str, int spaces) {
  QString result;
  auto lines = str.split('\n');

  for (int i = 0; i < lines.size(); ++i)
    result += QString(spaces, ' ') + lines[i] + "\n";

  return result;
}

} // namespace

MontageTemplateDialog::MontageTemplateDialog(OpenDataFile *file,
                                             QWidget *parent)
    : QDialog(parent), file(file),
      dir(MyApplication::makeAppSubdir({"montageTemplates"})) {
  readTemplates();

  QVBoxLayout *box = new QVBoxLayout();
  QSplitter *splitter = new QSplitter(Qt::Horizontal);
  setLayout(box);
  setWindowTitle("Montage Templates");
  resize(700, 700);

  // Set up the list.
  QVBoxLayout *vbox = new QVBoxLayout();
  QWidget *widget = new QWidget();
  widget->setLayout(vbox);
  QLabel *label = new QLabel("Templates:");
  label->setToolTip("XML files located in the 'montageTemplates' subdirectory");
  vbox->addWidget(label);

  list = new QListWidget();
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setContextMenuPolicy(Qt::ActionsContextMenu);
  populateList();
  connect(list, SIGNAL(currentRowChanged(int)), this,
          SLOT(showTemplateText(int)));
  vbox->addWidget(list);
  splitter->addWidget(widget);

  QAction *action = new QAction("Delete", this);
  action->setStatusTip("Remove selected templates");
  action->setShortcut(QKeySequence::Delete);
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()), this, SLOT(deleteTemplate()));
  list->addAction(action);

  action = new QAction("Rename", this);
  action->setStatusTip("Rename selected template");
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()), this, SLOT(renameTemplate()));
  list->addAction(action);

  // Set up preview text.
  vbox = new QVBoxLayout();
  widget = new QWidget();
  widget->setLayout(vbox);
  label = new QLabel("Preview:");
  vbox->addWidget(label);

  textEdit = new QTextEdit();
  textEdit->setReadOnly(true);
  textEdit->setWordWrapMode(QTextOption::NoWrap);
  vbox->addWidget(textEdit);
  splitter->addWidget(widget);

  box->addWidget(splitter);

  // Set up button box.
  auto addButton = new QPushButton("Add");
  addButton->setToolTip("Create a new montage from the selected template");
  connect(addButton, SIGNAL(clicked()), this, SLOT(addMontage()));

  auto saveButton = new QPushButton("Save Current");
  saveButton->setToolTip("Save the current montage as a template");
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveCurrentMontage()));

  auto buttonBox = new QDialogButtonBox(Qt::Horizontal);
  buttonBox->addButton(addButton, QDialogButtonBox::AcceptRole);
  buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);
  box->addWidget(buttonBox);
}

void MontageTemplateDialog::readTemplates() {
  for (const QFileInfo &e : dir.entryInfoList()) {
    if (e.isFile()) {
      TemplateFile tf;
      tf.fileName = e.fileName();
      tf.rows = readRows(e);
      templates.push_back(tf);
    }
  }
}

void MontageTemplateDialog::populateList() {
  int selectedIndex = list->currentRow();
  selectedIndex = max(0, selectedIndex);
  list->clear();
  QStringList strList;

  for (const TemplateFile &e : templates) {
    QFileInfo fi(e.fileName);
    strList.push_back(fi.baseName());
  }

  list->addItems(strList);

  if (selectedIndex < list->count())
    list->setCurrentRow(selectedIndex);
}

bool MontageTemplateDialog::getUniqueFileName(QString newName,
                                              QFileInfo *fileInfo) {
  while (1) {
    newName = QInputDialog::getText(this, "Montage template name",
                                    "Select new montage template name",
                                    QLineEdit::Normal, newName);

    if (newName.isNull())
      return false;

    *fileInfo = QFileInfo(dir.filePath(newName) + ".xml");

    if (!fileInfo->exists())
      return true;
  }
}

void MontageTemplateDialog::addMontage() {
  int selectedRow = list->currentRow();
  if (selectedRow < 0)
    return;

  const TemplateFile &selectedItem = templates[selectedRow];
  QFileInfo fi(selectedItem.fileName);
  string montageName = fi.baseName().toStdString();
  UndoCommandFactory *undoFactory = file->undoFactory;
  undoFactory->beginMacro("Add " + QString::fromStdString(montageName));

  const AbstractMontageTable *montageTable = file->dataModel->montageTable();
  int index = montageTable->rowCount();
  undoFactory->insertMontage(index);

  Montage m = montageTable->row(index);
  m.name = montageName;
  undoFactory->changeMontage(index, m);

  const AbstractTrackTable *trackTable = montageTable->trackTable(index);
  int trackCount = static_cast<int>(selectedItem.rows.size());
  undoFactory->insertTrack(index, 0, trackCount);

  for (int i = 0; i < trackCount; ++i) {
    Track t = trackTable->row(i);
    t.label = selectedItem.rows[i][0].toStdString();
    t.code = selectedItem.rows[i][1].toStdString();
    undoFactory->changeTrack(index, i, t);
  }

  undoFactory->endMacro();

  assert(0 <= index && index < file->dataModel->montageTable()->rowCount() &&
         "Make sure the selected index is legal");
  OpenDataFile::infoTable.setSelectedMontage(index);

  accept();
}

void MontageTemplateDialog::deleteTemplate() {
  int index = list->currentRow();
  if (index < 0)
    return;

  const QString &fileName = templates[index].fileName;
  bool res = dir.remove(fileName);

  if (!res)
    cerr << "Error while removing file '" << fileName.toStdString() << "'"
         << endl;
  assert(res && "Error while removing file");

  templates.erase(next(templates.begin(), index));
  populateList();
}

void MontageTemplateDialog::renameTemplate() {
  int index = list->currentRow();
  if (index < 0)
    return;

  QString fileName = templates[index].fileName;
  QFileInfo fi;
  bool res = getUniqueFileName(QFileInfo(fileName).baseName(), &fi);

  if (res) {
    QString newFileName = fi.fileName();
    res = dir.rename(fileName, newFileName);

    if (!res)
      cerr << "Error while renaming file from '" << fileName.toStdString()
           << "' to '" << newFileName.toStdString() << "'" << endl;
    assert(res && "Error while renaming file");

    templates[index].fileName = newFileName;
    populateList();
  }
}

void MontageTemplateDialog::saveCurrentMontage() {
  int selectedMontage = OpenDataFile::infoTable.getSelectedMontage();
  const AbstractTrackTable *trackTable =
      file->dataModel->montageTable()->trackTable(selectedMontage);

  TemplateFile tf;
  xml_document xmlFile;
  xml_node document = xmlFile.append_child("document");

  for (int i = 0; i < trackTable->rowCount(); ++i) {
    array<QString, 2> arrayRow;
    Track t = trackTable->row(i);
    xml_node row = document.append_child("row");

    arrayRow[0] = QString::fromStdString(t.label);
    row.append_attribute("label").set_value(t.label.c_str());

    arrayRow[1] = QString::fromStdString(t.code);
    row.append_child("code")
        .append_child(node_pcdata)
        .set_value(t.code.c_str());

    tf.rows.push_back(arrayRow);
  }

  QFileInfo fi;
  QString name =
      file->dataModel->montageTable()->row(selectedMontage).name.c_str();
  bool res = getUniqueFileName(name, &fi);

  if (res) {
    tf.fileName = fi.fileName();
    string path = fi.filePath().toStdString();

    if (!xmlFile.save_file(path.c_str())) {
      logToFileAndConsole("Error while writing file '" << path << "'.");
      assert(false && "Error while writing file");
    } else {
      templates.push_back(tf);
      populateList();
    }
  }
}

void MontageTemplateDialog::showTemplateText(int index) {
  QString text;

  if (0 <= index) {
    const TemplateFile &tf = templates[index];
    int maxLabelLength = 0;

    for (const auto &e : tf.rows)
      maxLabelLength = max(maxLabelLength, e[0].size());

    maxLabelLength += 3; // Additional spacing between columns.

    for (const auto &e : tf.rows) {
      QString str = indentString(e[1], maxLabelLength);
      text += e[0] + str.right(str.size() - e[0].size());
      text += "\n"; // An empty line between rows.
    }
  }

  textEdit->setText(text);
}
