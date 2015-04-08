#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

#include <map>
#include <utility>

class QTableView;
class QAbstractTableModel;
class QHBoxLayout;

class Manager : public QWidget
{
	Q_OBJECT

public:
	Manager(QWidget* parent = nullptr);
	virtual ~Manager();

	void setModel(QAbstractTableModel* model);

protected:
	QTableView* tableView;
	QHBoxLayout* buttonLayout;

private:
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
	void removeRow();
	void copy();
	void copyHtml();
	void paste();
};

#endif // MANAGER_H
