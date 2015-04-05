#ifndef MONTAGEMANAGER_H
#define MONTAGEMANAGER_H

#include <QWidget>

class QTableView;
class MontageTable;

class MontageManager : public QWidget
{
	Q_OBJECT

public:
	explicit MontageManager(QWidget* parent = nullptr);
	~MontageManager();

	void setModel(MontageTable* model);

private:
	QTableView* tableView;

private slots:
	void addRow();
	void removeRow();
};

#endif // MONTAGEMANAGER_H
