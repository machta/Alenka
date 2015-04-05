#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include <QMainWindow>

class DataFile;
class SignalViewer;
class TrackManager;
class EventManager;
class EventTypeManager;
class QComboBox;
class QCheckBox;
class QDockWidget;

class SignalFileBrowserWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SignalFileBrowserWindow(QWidget* parent = nullptr);
	~SignalFileBrowserWindow();

signals:

public slots:

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	DataFile* file = nullptr;
	SignalViewer* signalViewer;
	TrackManager* trackManager;
	EventManager* eventManager;
	EventTypeManager* eventTypeManager;
	QComboBox* lowpassComboBox;
	QComboBox* highpassComboBox;
	QCheckBox* notchCheckBox;
	QComboBox* montageComboBox;

private slots:
	void openFile();
	void closeFile();
	void saveFile();
	void lowpassComboBoxUpdate(const QString& text);
	void lowpassComboBoxUpdate(double value);
	void highpassComboBoxUpdate(const QString& text);
	void highpassComboBoxUpdate(double value);
	void updateManagers(int value);
};

#endif // SIGNALFILEBROWSERWINDOW_H
