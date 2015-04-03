#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include <QMainWindow>

class DataFile;
class SignalViewer;
class MontageManager;
class EventManager;
class EventTypeManager;
class QComboBox;
class QCheckBox;

class SignalFileBrowserWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SignalFileBrowserWindow(QWidget* parent = 0);
	~SignalFileBrowserWindow();

signals:

public slots:

private:
	DataFile* file = nullptr;
	SignalViewer* signalViewer;
	MontageManager* montageManager;
	EventManager* eventManager;
	EventTypeManager* eventTypeManager;
	QComboBox* lowpassComboBox;
	QComboBox* highpassComboBox;
	QCheckBox* notchCheckBox;

private slots:
	void openFile();
	void closeFile();
	void saveFile();
	void showMontageManager(bool checked);
	void showEventManager(bool checked);
	void showEventTypeManager(bool checked);
	void lowpassComboBoxUpdate(const QString& text);
	void lowpassComboBoxUpdate(double value);
	void highpassComboBoxUpdate(const QString& text);
	void highpassComboBoxUpdate(double value);

};

#endif // SIGNALFILEBROWSERWINDOW_H
