#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QWidget>

class QTableView;
class TrackTable;

class TrackManager : public QWidget
{
	Q_OBJECT

public:
	explicit TrackManager(QWidget* parent = nullptr);
	~TrackManager();

	void setModel(TrackTable* model);

private:
	QTableView* tableView;

private slots:
	void addRow();
	void removeRow();
};

#endif // TRACKMANAGER_H
