#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include <QMainWindow>

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

	void connectModelToUpdate(QAbstractTableModel* model);
	void horizontalZoom(double factor);
	void verticalZoom(double factor);

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
};

#endif // SIGNALFILEBROWSERWINDOW_H
