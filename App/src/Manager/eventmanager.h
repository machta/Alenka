#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "manager.h"

class DataFile;
class Canvas;

/**
 * @brief Event manager widget implementation.
 */
class EventManager : public Manager
{
	Q_OBJECT

public:
	explicit EventManager(QWidget* parent = nullptr);
	virtual ~EventManager();

	/**
	 * @brief Sets the pointer to Canvas.
	 */
	void setReferences(const Canvas* canvas)
	{
		this->canvas = canvas;
	}

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(DataFile* file)
	{
		this->file = file;
	}

private:
	const Canvas* canvas = nullptr;
	DataFile* file = nullptr;

private slots:
	/**
	 * @brief Change the position of the SignalViewer to show the event on the selected row.
	 */
	void goToEvent();
};

#endif // EVENTMANAGER_H
