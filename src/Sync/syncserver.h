#ifndef SYNCSERVER_H
#define SYNCSERVER_H

#include <QObject>

#include <memory>
#include <vector>

class QWebSocketServer;
class QWebSocket;

/**
 * @brief This class manages a WebSocket server stack and takes care of
 * server-client connections.
 */
class SyncServer : public QObject {
  Q_OBJECT

  std::unique_ptr<QWebSocketServer> server;
  std::vector<std::unique_ptr<QWebSocket>> sockets;

public:
  explicit SyncServer(QObject *parent = nullptr);
  ~SyncServer() override;

  int launch(int port);
  int shutDown();
  int connectionCount() const { return static_cast<int>(sockets.size()); }

signals:
  void messageReceived(const QByteArray &message);

public slots:
  int sendMessage(const QByteArray &message);

private slots:
  void broadcastMessage(const QByteArray &message);
};

#endif // SYNCSERVER_H
