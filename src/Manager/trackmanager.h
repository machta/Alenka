#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include "manager.h"

/**
 * @brief Track manager widget implementation.
 */
class TrackManager : public Manager {
  Q_OBJECT

public:
  explicit TrackManager(QWidget *parent = nullptr) : Manager(parent) {}

protected slots:
  bool insertRowBack() override;
};

#endif // TRACKMANAGER_H
