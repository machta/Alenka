#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include <QMainWindow>

#include <functional>

class DataFile;
class SignalViewer;
class TrackManager;
class EventManager;
class EventTypeManager;
class MontageManager;
class QComboBox;
class QCheckBox;
class QDockWidget;
class QAbstractTableModel;
class QActionGroup;
class QLabel;
class QAction;

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
	MontageManager* montageManager;
	QComboBox* lowpassComboBox;
	QComboBox* highpassComboBox;
	QCheckBox* notchCheckBox;
	QComboBox* montageComboBox;
	QComboBox* eventTypeComboBox;
	QActionGroup* timeModeActionGroup;
	QActionGroup* timeLineIntervalActionGroup;
	QAction* setTimeLineIntervalAction;
	QLabel* timeModeStatusLabel;
	QLabel* timeStatusLabel;
	QLabel* positionStatusLabel;
	QLabel* cursorStatusLabel;

	void connectModel(QAbstractTableModel* model, std::function<void ()> f);
	void horizontalZoom(double factor);
	void verticalZoom(double factor);
	void mode(int m);

private slots:
	void openFile();
	void closeFile();
	void saveFile();
	void lowpassComboBoxUpdate(const QString& text);
	void lowpassComboBoxUpdate(double value);
	void highpassComboBoxUpdate(const QString& text);
	void highpassComboBoxUpdate(double value);
	void updateManagers(int value);
	void horizontalZoomIn();
	void horizontalZoomOut();
	void verticalZoomIn();
	void verticalZoomOut();
	void timeMode0()
	{
		mode(0);
	}
	void timeMode1()
	{
		mode(1);
	}
	void timeMode2()
	{
		mode(2);
	}
	void updateTimeMode(int mode);
	void updatePositionStatusLabel();
	void updateCursorStatusLabel();
	void updateMontageComboBox();
	void updateEventTypeComboBox();
};

#endif // SIGNALFILEBROWSERWINDOW_H
