#include "syncserver.h"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <cassert>
#include <iostream>

using namespace std;

namespace {

auto deleteClosedSockets(vector<unique_ptr<QWebSocket>> &sockets) {
  vector<unique_ptr<QWebSocket>> active;

  for (auto &e : sockets) {
    if (e->isValid())
      active.push_back(move(e));
  }

  return active;
}

} // namespace

void sendMessageThroughSocket(const QByteArray &message, QWebSocket *socket) {
  auto bytesSent = socket->sendBinaryMessage(message);
  (void)bytesSent;
  assert(bytesSent == message.size() && "Server failed to send the message.");
}

SyncServer::SyncServer(QObject *parent)
    : QObject(parent),
      server(new QWebSocketServer("", QWebSocketServer::NonSecureMode)) {
  connect(server.get(), &QWebSocketServer::newConnection, [this]() {
    auto socket = unique_ptr<QWebSocket>(server->nextPendingConnection());
    assert(socket);

    connect(socket.get(), SIGNAL(binaryMessageReceived(QByteArray)), this,
            SIGNAL(messageReceived(QByteArray)));
    connect(socket.get(), SIGNAL(binaryMessageReceived(QByteArray)), this,
            SLOT(broadcastMessage(QByteArray)));
    sockets.push_back(move(socket));
  });
}

SyncServer::~SyncServer() { shutDown(); }

int SyncServer::launch(int port) {
  bool result = server->listen(QHostAddress::Any, port);
  return result ? 0 : 1;
}

int SyncServer::shutDown() {
  sockets.clear();

  if (server->isListening())
    server->close();

  return 0;
}

int SyncServer::sendMessage(const QByteArray &message) {
  sockets = deleteClosedSockets(sockets);

  for (auto &e : sockets) {
    sendMessageThroughSocket(message, e.get());
  }

  return 0;
}

void SyncServer::broadcastMessage(const QByteArray &message) {
  QObject *signalSender = sender();

  sockets = deleteClosedSockets(sockets);

  for (auto &e : sockets) {
    if (e.get() != signalSender)
      sendMessageThroughSocket(message, e.get());
  }
}
