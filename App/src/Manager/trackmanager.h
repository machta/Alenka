#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include "manager.h"

class TrackManager : public Manager
{
	Q_OBJECT

public:
	explicit TrackManager(QWidget* parent = nullptr);
	virtual ~TrackManager();
};

#endif // TRACKMANAGER_H
