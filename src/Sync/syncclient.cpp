#include "syncclient.h"

#include <QtWebSockets/QWebSocket>

#include <cassert>

SyncClient::~SyncClient()
{
	disconnect();
	delete socket;
}

void SyncClient::connectToServer(QUrl url, int port)
{
	assert(socket == nullptr);
	assert(url.isValid());

	socket = new QWebSocket();

	connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
	connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));

	url.setPort(port);
	socket->open(url);
}

void SyncClient::disconnect()
{
	if (socket)
	{
		socket->close();
		delete socket;
		socket = nullptr;
	}
}

int SyncClient::sendMessage(const QByteArray& message)
{
	assert(socket != nullptr);

	if (socket->isValid())
	{
		socket->sendBinaryMessage(message);
		return 0;
	}
	else
	{
		return 1;
	}
}
