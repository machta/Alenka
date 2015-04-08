#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "manager.h"

class EventManager : public Manager
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = nullptr);
	virtual ~EventManager();
};

#endif // EVENTMANAGER_H
