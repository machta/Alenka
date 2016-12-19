#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

#include <map>
#include <utility>

class QTableView;
class QAbstractTableModel;
class QGridLayout;
class QPushButton;

/**
 * @brief The base class of the manager widgets.
 */
class Manager : public QWidget
{
	Q_OBJECT

public:
	Manager(QWidget* parent = nullptr);
	virtual ~Manager();

	/**
	 * @brief Associates a table class with this manager.
	 * @param model [in]
	 */
	void setModel(QAbstractTableModel* model);

protected:
	QTableView* tableView;

	/**
	 * @brief Adds the button to the prepared layout on top of the widget.
	 */
	void addButton(QPushButton* button);

private:
	int buttonsPerRow = 4;
	int buttonsAdded = 0;
	QGridLayout* buttonLayout;

	/**
	 * @brief Returns the selected cells as a ordered data structure.
	 *
	 * The key for accessing the cells in the map is an ordered pair (row, column).
	 */
	std::map<std::pair<int, int>, QString> textOfSelection();

	/**
	 * @brief Replaces tab and new line characters with spaces.
	 */
	QString replaceTabsAndBreaks(const QString& string)
	{
		QString newString;
		for (const auto& e : string)
		{
			newString.push_back(e == '\t' || e == '\n' ? ' ' : e);
		}
		return newString;
	}

private slots:
	/**
	 * @brief Appends a row to the end of the table.
	 */
	void addRow();

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
};

#endif // MANAGER_H
