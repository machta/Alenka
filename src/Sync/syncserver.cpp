#include "syncserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <cassert>
#include <iostream>

using namespace std;

SyncServer::~SyncServer()
{
	shutDown();

	for (QWebSocket* e : sockets)
		delete e;

	delete server;
}

int SyncServer::launch(int port)
{
	assert(server == nullptr);

	server = new QWebSocketServer("", QWebSocketServer::NonSecureMode);

	bool result = server->listen(QHostAddress::Any, port);

	if (!result)
		return 1;

	connect(server, &QWebSocketServer::newConnection, [this] () {
		auto socket = server->nextPendingConnection();
		assert(socket);

		sockets.push_back(socket);
		connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
	});

	return 0;
}

int SyncServer::shutDown()
{
	if (server)
	{
		server->close();
		delete server;
		server = nullptr;
	}
	return 0;
}

int SyncServer::sendMessage(const QByteArray& message)
{
	if (server == nullptr)
		return 0;

	sockets = deleteClosedSockets(sockets);

	for (QWebSocket* e : sockets)
	{
		auto bytesSent = e->sendBinaryMessage(message);
		assert(bytesSent == message.size());
	}

	return 0;
}

vector<QWebSocket*> SyncServer::deleteClosedSockets(const vector<QWebSocket*>& sockets)
{
	vector<QWebSocket*> active;
	for (QWebSocket* e : sockets)
	{
		if(e->isValid())
		{
			active.push_back(e);
		}
		else
		{
			delete e;
			disconnect(e, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
		}
	}
	return active;
}
