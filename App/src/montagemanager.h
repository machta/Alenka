#ifndef MONTAGEMANAGER_H
#define MONTAGEMANAGER_H

#include <QDialog>

#include <QDialog>

#include "DataFile/montagetable.h"

namespace Ui
{
class MontageManager;
}

class MontageManager : public QDialog
{
	Q_OBJECT

public:
	explicit MontageManager(QWidget* parent = 0);
	~MontageManager();

	void setModel(MontageTable* model);

private:
	Ui::MontageManager* ui;
};

#endif // MONTAGEMANAGER_H
