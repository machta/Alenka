#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>

#include "signalviewer.h"
#include "eventtypemanager.h"
#include "DataFile/gdf2.h"

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

private:
	Ui::TestWindow* ui;

	DataFile* file = nullptr;
};

#endif // TESTWINDOW_H
