#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "manager.h"

class Canvas;

/**
 * @brief Event manager widget implementation.
 */
class EventManager : public Manager {
  Q_OBJECT

public:
  explicit EventManager(QWidget *parent = nullptr);

  /**
   * @brief Sets the pointer to Canvas.
   */
  void setReferences(const Canvas *canvas) { this->canvas = canvas; }

protected slots:
  bool insertRowBack() override;

private:
  const Canvas *canvas = nullptr;

private slots:
  /**
   * @brief Change the position of the SignalViewer to show the event on the
   * selected row.
   */
  void goToEvent();
};

#endif // EVENTMANAGER_H
