#ifndef SYNCSERVER_H
#define SYNCSERVER_H

#include <QObject>

#include <vector>

class QWebSocketServer;
class QWebSocket;

class SyncServer : public QObject
{
	Q_OBJECT

public:
	explicit SyncServer(QObject* parent = nullptr);
	~SyncServer();

	int launch(int port);
	int shutDown();
	int connectionCount() const
	{
		return static_cast<int>(sockets.size());
	}

signals:
	void messageReceived(const QByteArray& message);

public slots:
	int sendMessage(const QByteArray& message);

private:
	QWebSocketServer* server;
	std::vector<QWebSocket*> sockets;

	std::vector<QWebSocket*> deleteClosedSockets(const std::vector<QWebSocket*>& sockets);
	void closeSocket(QWebSocket* socket);

private slots:
	void broadcastMessage(const QByteArray& message);
};

#endif // SYNCSERVER_H
