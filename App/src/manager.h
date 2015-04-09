#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

#include <map>
#include <utility>

class QTableView;
class QAbstractTableModel;
class QGridLayout;
class QPushButton;

class Manager : public QWidget
{
	Q_OBJECT

public:
	Manager(QWidget* parent = nullptr);
	virtual ~Manager();

	void setModel(QAbstractTableModel* model);

protected:
	QTableView* tableView;

	void addButton(QPushButton* button);

private:
	int buttonsPerRow = 4;
	int buttonsAdded = 0;
	QGridLayout* buttonLayout;

	std::map<std::pair<int, int>, QString> textOfSelection();
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
	void addRow();
	void removeRows();
	void copy();
	void copyHtml();
	void paste();
};

#endif // MANAGER_H
