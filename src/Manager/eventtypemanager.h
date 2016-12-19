#ifndef EVENTTYPEMANAGER_H
#define EVENTTYPEMANAGER_H

#include "manager.h"

/**
 * @brief Event type manager widget implementation.
 */
class EventTypeManager : public Manager
{
	Q_OBJECT

public:
	explicit EventTypeManager(QWidget* parent = nullptr);
	virtual ~EventTypeManager();
};

#endif // EVENTTYPEMANAGER_H
