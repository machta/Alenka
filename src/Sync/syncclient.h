#ifndef SYNCCLIENT_H
#define SYNCCLIENT_H

#include <QObject>

class QWebSocket;

class SyncClient : public QObject
{
	Q_OBJECT

public:
	explicit SyncClient(QObject* parent = nullptr) : QObject(parent)
	{}
	~SyncClient();
	void connectToServer(QUrl url, int port);
	void disconnect();

signals:
	void messageReceived(const QByteArray& message);
	void disconnected();

public slots:
	int sendMessage(const QByteArray& message);

private:
	QWebSocket* socket = nullptr;
};

#endif // SYNCCLIENT_H
