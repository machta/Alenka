#ifndef SYNCCLIENT_H
#define SYNCCLIENT_H

#include <QObject>

class QWebSocket;

class SyncClient : public QObject {
  Q_OBJECT

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

private:
  QWebSocket *socket;
};

#endif // SYNCCLIENT_H
