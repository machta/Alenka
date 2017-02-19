#include "syncclient.h"

#include <QtWebSockets/QWebSocket>

#include <cassert>
#include <iostream>

using namespace std;

SyncClient::SyncClient(QObject* parent) : QObject(parent)
{
	socket = new QWebSocket();
}

SyncClient::~SyncClient()
{
	disconnectServer();
	delete socket;
}

int SyncClient::connectServer(QUrl url, int port)
{
	assert(url.isValid());

	cerr << "Error " << socket->error() << " " << socket->errorString().toStdString() << endl;

	connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));	
	connect(socket, SIGNAL(disconnected()), this, SIGNAL(serverDisconnected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectServer()));

	connect(socket, &QWebSocket::connected, [] () {
		cerr << "Client connected" << endl;
	});

	url.setPort(port);
	socket->open(url);

	cerr << "Error " << socket->error() << " " << socket->errorString().toStdString() << endl;

	//return socket->isValid() ? 0 : 1;
	return 0;
}

void SyncClient::disconnectServer()
{
	if (socket->isValid())
	{
		socket->close();
	}
}

int SyncClient::sendMessage(const QByteArray& message)
{
	if (!socket->isValid())
		return 0;

	if (socket->isValid())
	{
		auto bytesSent = socket->sendBinaryMessage(message);
		assert(bytesSent == message.size() && "Client failed to send the message.");
		return 0;
	}
	else
	{
		return 1;
	}
}
