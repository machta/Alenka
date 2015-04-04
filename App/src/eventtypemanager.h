#ifndef EVENTTYPEMANAGER_H
#define EVENTTYPEMANAGER_H

#include <QWidget>

class QTableView;
class EventTypeTable;

class EventTypeManager : public QWidget
{
	Q_OBJECT

public:
	explicit EventTypeManager(QWidget* parent = nullptr);
	~EventTypeManager();

	void setModel(EventTypeTable* model);

private:
	QTableView* tableView;

private slots:
	void addRow();
	void removeRow();
};

#endif // EVENTTYPEMANAGER_H
