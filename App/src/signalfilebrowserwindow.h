#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include <QMainWindow>

#include "signalviewer.h"
#include "DataFile/gdf2.h"
#include "montagemanager.h"
#include "eventmanager.h"
#include "eventtypemanager.h"

class SignalFileBrowserWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SignalFileBrowserWindow(QWidget* parent = 0);
	~SignalFileBrowserWindow();

signals:

public slots:

private slots:
	void openFile();
	void closeFile();
	void showMontageManager();
	void showEventManager();
	void showEventTypeManager();

private:
	DataFile* file = nullptr;
	SignalViewer* signalViewer;
	MontageManager* montageManager;
	EventManager* eventManager;
	EventTypeManager* eventTypeManager;

};

#endif // SIGNALFILEBROWSERWINDOW_H
