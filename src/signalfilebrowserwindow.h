#ifndef SIGNALFILEBROWSERWINDOW_H
#define SIGNALFILEBROWSERWINDOW_H

#include "DataModel/infotable.h"

#include <QMainWindow>

#include <functional>
#include <vector>

namespace AlenkaFile
{
class DataFile;
class DataModel;
}
class OpenDataFile;
class SignalViewer;
class TrackManager;
class EventManager;
class EventTypeManager;
class MontageManager;
class QComboBox;
class QCheckBox;
class QAbstractTableModel;
class QActionGroup;
class QLabel;
class QAction;
class SpikedetAnalysis;
class SyncServer;
class SyncClient;
class SyncDialog;
class TableModel;
class DataModelVitness;
class QTimer;
class QUndoStack;
class UndoCommandFactory;

/**
 * @brief This class implements the top level window of the program.
 */
class SignalFileBrowserWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SignalFileBrowserWindow(QWidget* parent = nullptr);
	~SignalFileBrowserWindow();

	static QDateTime sampleToDate(AlenkaFile::DataFile* file, int sample);
	static QDateTime sampleToOffset(AlenkaFile::DataFile* file, int sample);
	static QString sampleToDateTimeString(AlenkaFile::DataFile* file, int sample, InfoTable::TimeMode mode = InfoTable::TimeMode::size);

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	AlenkaFile::DataFile* file = nullptr;
	OpenDataFile* openDataFile;
	AlenkaFile::DataModel* dataModel = nullptr;
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
	SpikedetAnalysis* spikedetAnalysis;
	double eventDuration = 0.1;
	SyncServer* syncServer;
	SyncClient* syncClient;
	SyncDialog* syncDialog;
	const int lastPositionReceivedDefault = -1000*1000*1000;
	int lastPositionReceived = lastPositionReceivedDefault;
	QAction* synchronize;
	TableModel* eventTypeTable = nullptr;
	TableModel* montageTable = nullptr;
	TableModel* eventTable = nullptr;
	TableModel* trackTable = nullptr;
	std::vector<QMetaObject::Connection> openFileConnections;
	QTimer* autoSaveTimer;
	std::string autoSaveName;
	QUndoStack* undoStack;
	UndoCommandFactory* undoFactory = nullptr;
	QAction* saveFileAction;

	void connectVitness(const DataModelVitness* vitness, std::function<void ()> f);
	void mode(int m);
	bool shouldSynchronizeView();
	void deleteAutoSave();

private slots:
	void openFile();
	void closeFile();
	void saveFile();
	void lowpassComboBoxUpdate(const QString& text);
	void lowpassComboBoxUpdate(double value);
	void highpassComboBoxUpdate(const QString& text);
	void highpassComboBoxUpdate(double value);
	void updateManagers(int value);
	void updateTimeMode(int mode);
	void updatePositionStatusLabel();
	void updateCursorStatusLabel();
	void updateMontageComboBox();
	void updateEventTypeComboBox();
	void runSpikedet();
	void receiveSyncMessage(const QByteArray& message);
	void sendSyncMessage();
	void cleanChanged(bool clean);
	void closeFileDestroy();
};

#endif // SIGNALFILEBROWSERWINDOW_H
