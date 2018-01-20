#ifndef SYNCCLIENT_H
#define SYNCCLIENT_H

#include <QObject>
#include <memory>

class QWebSocket;

/**
 * @brief Manages clinet-server connections via WebSocket.
 */
class SyncClient : public QObject {
  Q_OBJECT

  std::unique_ptr<QWebSocket> socket;

public:
  explicit SyncClient(QObject *parent = nullptr);
  ~SyncClient() override;

  int connectServer(QUrl url, int port);
  bool isValid();

signals:
  void messageReceived(const QByteArray &message);
  void serverDisconnected();

public slots:
  int sendMessage(const QByteArray &message);
  void disconnectServer();
};

#endif // SYNCCLIENT_H
