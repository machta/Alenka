#ifndef MONTAGEMANAGER_H
#define MONTAGEMANAGER_H

#include "manager.h"

class MontageManager : public Manager
{
	Q_OBJECT

public:
	explicit MontageManager(QWidget* parent = nullptr);
	virtual ~MontageManager();
};

#endif // MONTAGEMANAGER_H
