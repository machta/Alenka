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

/**
 * @brief This class implements the top level window of the program.
 */
class SignalFileBrowserWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SignalFileBrowserWindow(QWidget* parent = nullptr);
	~SignalFileBrowserWindow();

	static InfoTable infoTable;
	static QColor array2color(unsigned char* c)
	{
		QColor color;
		color.setRgb(c[0], c[1], c[2]);
		return color;
	}
	static void color2array(const QColor& color, unsigned char* c)
	{
		c[0] = color.red();
		c[1] = color.green();
		c[2] = color.blue();
	}
	static QDateTime sampleToDate(AlenkaFile::DataFile* file, int sample);
	static QDateTime sampleToOffset(AlenkaFile::DataFile* file, int sample);
	static QString sampleToDateTimeString(AlenkaFile::DataFile* file, int sample, InfoTable::TimeMode mode = InfoTable::TimeMode::size);

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	AlenkaFile::DataFile* file = nullptr;
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
	AlenkaFile::DataModel* dataModel = nullptr;
	std::vector<QMetaObject::Connection> openFileConnections;

	void connectVitness(DataModelVitness* vitness, std::function<void ()> f);
	void horizontalZoom(double factor);
	void verticalZoom(double factor);
	void mode(int m);
	bool shouldSynchronizeView();

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
	void runSpikedet();
	void receiveSyncMessage(const QByteArray& message);
	void sendSyncMessage();
};

#endif // SIGNALFILEBROWSERWINDOW_H
