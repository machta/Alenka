#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QWidget>

class EventTable;
class QTableView;

class EventManager : public QWidget
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = nullptr);
	~EventManager();

	void setModel(EventTable* model);

private:
	QTableView* tableView;

private slots:
	void addRow();
	void removeRow();
};

#endif // EVENTMANAGER_H
