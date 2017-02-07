#include "syncclient.h"

#include <QtWebSockets/QWebSocket>

#include <cassert>
#include <iostream>

using namespace std;

SyncClient::~SyncClient()
{
	disconnect();
	delete socket;
}

int SyncClient::connectToServer(QUrl url, int port)
{
	assert(socket == nullptr);
	assert(url.isValid());

	socket = new QWebSocket();

	cerr << "Error " << socket->error() << " " << socket->errorString().toStdString() << endl;

	connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
	connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	connect(socket, &QWebSocket::connected, [] () {
		cerr << "Client connected" << endl;
	});

	url.setPort(port);
	socket->open(url);

	cerr << "Error " << socket->error() << " " << socket->errorString().toStdString() << endl;

	//return socket->isValid() ? 0 : 1;
	return 0;
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
	if (socket == nullptr)
		return 0;

	assert(socket != nullptr);

	if (socket->isValid())
	{
		auto bytesSent = socket->sendBinaryMessage(message);
		assert(bytesSent == message.size());
		return 0;
	}
	else
	{
		return 1;
	}
}
