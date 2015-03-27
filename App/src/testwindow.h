#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>

#include "signalviewer.h"
#include "DataFile/gdf2.h"
#include "montagemanager.h"
#include "eventmanager.h"
#include "eventtypemanager.h"

namespace Ui
{
class TestWindow;
}

class TestWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TestWindow(QWidget* parent = 0);
	~TestWindow();

private slots:
	void on_actionEventTypeManager_triggered();
	void on_actionOpenFile_triggered();
	void on_actionCloseFile_triggered();
	void on_actionEventManager_triggered();
	void on_actionMontageManager_triggered();

private:
	Ui::TestWindow* ui;

	DataFile* file = nullptr;
	MontageManager* montageManager;
	EventManager* eventManager;
	EventTypeManager* eventTypeManager;
};

#endif // TESTWINDOW_H
