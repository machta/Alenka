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
	explicit EventTypeManager(EventTypeTable* model, QWidget* parent = 0);
	~EventTypeManager();

private:
	Ui::EventTypeManager* ui;
};

#endif // EVENTTYPEMANAGER_H
