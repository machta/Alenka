#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "manager.h"

class DataFile;
class Canvas;

class EventManager : public Manager
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = nullptr);
	virtual ~EventManager();

	void setReferences(const Canvas* canvas)
	{
		this->canvas = canvas;
	}
	void changeFile(DataFile* file)
	{
		this->file = file;
	}

private:
	const Canvas* canvas = nullptr;
	DataFile* file = nullptr;

private slots:
	void goToEvent();
};

#endif // EVENTMANAGER_H
