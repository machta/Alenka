#ifndef EVENTTYPEMANAGER_H
#define EVENTTYPEMANAGER_H

#include <QDialog>

#include "DataFile/eventtypetable.h"

namespace Ui
{
class EventTypeManager;
}

class EventTypeManager : public QDialog
{
	Q_OBJECT

public:
	explicit EventTypeManager(QWidget* parent = 0);
	~EventTypeManager();

	void setModel(EventTypeTable* model);

private:
	Ui::EventTypeManager* ui;
};

#endif // EVENTTYPEMANAGER_H
