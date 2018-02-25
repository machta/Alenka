#include "eventmanager.h"

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"
#include "../canvas.h"

#include <QAction>
#include <QPushButton>
#include <QTableView>

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

EventManager::EventManager(QWidget *parent) : Manager(parent) {
  QAction *goToAction = new QAction("Go to Event", this);
  goToAction->setStatusTip(
      "Scroll to the position of the event on the selected row");
  goToAction->setShortcut(QKeySequence("Ctrl+g"));
  goToAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(goToAction, SIGNAL(triggered()), this, SLOT(goToEvent()));
  tableView->addAction(goToAction);

  QPushButton *goToButton = new QPushButton("Go to Event", this);
  connect(goToButton, SIGNAL(clicked()), this, SLOT(goToEvent()));
  addButton(goToButton);
}

bool EventManager::insertRowBack() {
  if (file && 0 < file->dataModel->montageTable()->rowCount()) {
    int rc = file->dataModel->montageTable()
                 ->eventTable(OpenDataFile::infoTable.getSelectedMontage())
                 ->rowCount();
    file->undoFactory->insertEvent(OpenDataFile::infoTable.getSelectedMontage(),
                                   rc, 1, "add Event row");
    return true;
  }

  return false;
}

// TODO: Don't constrain goto to the dimensions of the slider, but rather make
// sure time line is always correctly positioned.
void EventManager::goToEvent() {
  if (!file)
    return;

  auto currentIndex = tableView->selectionModel()->currentIndex();

  if (file && currentIndex.isValid()) {
    InfoTable &it = OpenDataFile::infoTable;
    int col = static_cast<int>(Event::Index::position);
    auto index = tableView->model()->index(currentIndex.row(), col);

    int position = tableView->model()->data(index, Qt::EditRole).toInt();
    position =
        qBound(0, position, static_cast<int>(file->file->getSamplesRecorded()));
    it.setPosition(position);
  }
}
