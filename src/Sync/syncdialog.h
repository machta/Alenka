#ifndef SYNCDIALOG_H
#define SYNCDIALOG_H

#include <QDialog>

#include <memory>

class OpenDataFile;
class SyncServer;
class SyncClient;

class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @brief This class implements the dialog window used for managing
 * timeline-synchronization connections.
 */
class SyncDialog : public QDialog {
  Q_OBJECT

  OpenDataFile *file = nullptr;
  std::unique_ptr<SyncServer> server;
  std::unique_ptr<SyncClient> client;
  const int lastPositionReceivedDefault = -1000'000'000;
  int lastPositionReceived = lastPositionReceivedDefault;
  bool shouldSynchronize = true;

  QComboBox *combo;
  QWidget *serverControls;
  QWidget *clientControls;
  QLineEdit *serverPortEdit;
  QLineEdit *clientPortEdit;
  QLineEdit *clientIpEdit;
  QPushButton *launchButton;
  QPushButton *connectButton;
  QLabel *serverStatus;
  QLabel *clientStatus;

public:
  explicit SyncDialog(QWidget *parent = nullptr);

  void changeFile(OpenDataFile *file) { this->file = file; }

public slots:
  void setShouldSynchronize(bool value) {
    if (!(shouldSynchronize = value))
      lastPositionReceived = lastPositionReceivedDefault;
  }

private:
  void buildServerControls();
  void buildClientControls();

private slots:
  void activateControls(const QString &text);
  void launchServer();
  void shutDownServer();
  void connectClient();
  void disconnectClient();
  void changeEnableControls(bool enable);

  void receiveSyncMessage(const QByteArray &message);
  void sendSyncMessage();
};

#endif // SYNCDIALOG_H
