#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QDialog>

#include "DataFile/tracktable.h"

namespace Ui
{
class TrackManager;
}

class TrackManager : public QDialog
{
	Q_OBJECT

public:
	explicit TrackManager(QWidget* parent = 0);
	~TrackManager();

	void setModel(TrackTable* model);

private:
	Ui::TrackManager* ui;
};

#endif // TRACKMANAGER_H
