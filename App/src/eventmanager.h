#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "manager.h"

class DataFile;

class EventManager : public Manager
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = nullptr);
	virtual ~EventManager();

	void changeFile(DataFile* file)
	{
		this->file = file;
	}

private:
	DataFile* file = nullptr;

private slots:
	void goToEvent();
};

#endif // EVENTMANAGER_H
