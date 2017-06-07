#ifndef MONTAGEMANAGER_H
#define MONTAGEMANAGER_H

#include "manager.h"

/**
 * @brief Montage manager widget implementation.
 */
class MontageManager : public Manager {
  Q_OBJECT

public:
  explicit MontageManager(QWidget *parent = nullptr) : Manager(parent) {}

protected slots:
  bool insertRowBack() override;
};

#endif // MONTAGEMANAGER_H
