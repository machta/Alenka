#include "syncserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <cassert>
#include <iostream>

using namespace std;

void sendMessageThroughSocket(const QByteArray& message, QWebSocket* socket)
{
	auto bytesSent = socket->sendBinaryMessage(message);
	assert(bytesSent == message.size() && "Server failed to send the message.");
}

SyncServer::SyncServer(QObject* parent) : QObject(parent)
{
	server = new QWebSocketServer("", QWebSocketServer::NonSecureMode);

	connect(server, &QWebSocketServer::newConnection, [this] () {
		auto socket = server->nextPendingConnection();
		assert(socket);

		sockets.push_back(socket);
		connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
		connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(broadcastMessage(QByteArray)));
	});
}

SyncServer::~SyncServer()
{
	shutDown();
	delete server;
}

int SyncServer::launch(int port)
{
	bool result = server->listen(QHostAddress::Any, port);
	return result ? 0 : 1;
}

int SyncServer::shutDown()
{
	for (QWebSocket* e : sockets)
		closeSocket(e);
	sockets.clear();

	if (server->isListening())
		server->close();

	return 0;
}

int SyncServer::sendMessage(const QByteArray& message)
{
	sockets = deleteClosedSockets(sockets);

	for (QWebSocket* e : sockets)
	{
		sendMessageThroughSocket(message, e);
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
			closeSocket(e);
		}
	}
	return active;
}

void SyncServer::closeSocket(QWebSocket* socket)
{
	disconnect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
	disconnect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(broadcastMessage(QByteArray)));
	delete socket;
}

void SyncServer::broadcastMessage(const QByteArray& message)
{
	QObject* signalSender = sender();

	sockets = deleteClosedSockets(sockets);

	for (QWebSocket* e : sockets)
	{
		if (e != signalSender)
			sendMessageThroughSocket(message, e);
	}
}
