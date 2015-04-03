#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QDialog>

#include "DataFile/eventtable.h"

namespace Ui
{
class EventManager;
}

class EventManager : public QDialog
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = 0);
	~EventManager();

	void setModel(EventTable* model);

private:
	Ui::EventManager* ui;

private slots:
	void on_addRowButton_clicked();

	void on_removeRowButton_clicked();
};

#endif // EVENTMANAGER_H
