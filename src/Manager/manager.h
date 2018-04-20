#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

#include <map>
#include <utility>

class QTableView;
class TableModel;
class QGridLayout;
class QPushButton;
class OpenDataFile;

// TODO: Possibly merge Manager and DataModel dirs.
/**
 * @brief The base class of the manager widgets.
 */
class Manager : public QWidget {
  Q_OBJECT

  const int buttonsPerRow = 4;
  int buttonsAdded = 0;
  QGridLayout *buttonLayout;

public:
  explicit Manager(QWidget *parent = nullptr);

  /**
   * @brief Associates a table class with this manager.
   * @param model [in]
   */
  void setModel(TableModel *model);

  /**
   * @brief Notifies this object that the DataFile changed.
   * @param file Pointer to the data file. nullptr means file was closed.
   */
  void changeFile(OpenDataFile *file) { this->file = file; }

protected:
  QTableView *tableView;
  OpenDataFile *file = nullptr;

  /**
   * @brief Adds the button to the prepared layout on top of the widget.
   */
  void addButton(QPushButton *button);

protected slots:
  /**
   * @brief Appends a row to the end of the table.
   */
  virtual bool insertRowBack() = 0;

private:
  /**
   * @brief Returns the selected cells as a ordered data structure.
   *
   * The key for accessing the cells in the map is an ordered pair (row,
   * column).
   */
  std::map<std::pair<int, int>, QString> textOfSelection();

  void addSeparator();
  std::vector<int> reverseSortedSelectedRows();
  bool askToDeleteRows(int rowCount);
  void pasteSingleCell(const std::string &cell);
  void pasteBlock(const std::vector<std::vector<std::string>> &cells);

private slots:
  /**
   * @brief Removes selected rows from the table.
   */
  void removeRows();

  /**
   * @brief Copies the selected cells to clipboard.
   *
   * The some characters in the cells are replaced by spaces to ensure
   * the proper format -- tabs delimiting cells, newlines rows.
   */
  void copy();

  /**
   * @brief Copies the selected cells to clipboard as a HTML table.
   *
   * The content of the cells is copied as is.
   */
  void copyHtml();

  /**
   * @brief Paste the table from clipboard starting from the current cell.
   *
   * New rows are added as needed. If the table is not complete, default values
   * are used for the extra cells.
   */
  void paste();

  void setColumn();
};

#endif // MANAGER_H
