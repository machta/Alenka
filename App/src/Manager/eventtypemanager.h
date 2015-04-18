#ifndef EVENTTYPEMANAGER_H
#define EVENTTYPEMANAGER_H

#include "manager.h"

class EventTypeManager : public Manager
{
	Q_OBJECT

public:
	explicit EventTypeManager(QWidget* parent = nullptr);
	virtual ~EventTypeManager();
};

#endif // EVENTTYPEMANAGER_H
