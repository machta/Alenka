#include "syncclient.h"

#include <QtWebSockets/QWebSocket>

#include <cassert>
#include <iostream>

using namespace std;

SyncClient::SyncClient(QObject *parent)
    : QObject(parent), socket(new QWebSocket()) {}

SyncClient::~SyncClient() { disconnectServer(); }

int SyncClient::connectServer(QUrl url, int port) {
  assert(url.isValid());

  // TODO: Redirect these error messages to log or drop them altogether.
  cerr << "Error " << socket->error() << " "
       << socket->errorString().toStdString() << endl;

  connect(socket.get(), SIGNAL(binaryMessageReceived(QByteArray)), this,
          SIGNAL(messageReceived(QByteArray)));
  connect(socket.get(), SIGNAL(disconnected()), this,
          SIGNAL(serverDisconnected()));
  connect(socket.get(), SIGNAL(disconnected()), this, SLOT(disconnectServer()));

  connect(socket.get(), &QWebSocket::connected,
          []() { cerr << "Client connected" << endl; });

  url.setPort(port);
  socket->open(url);

  cerr << "Error " << socket->error() << " "
       << socket->errorString().toStdString() << endl;

  return 0;
}

bool SyncClient::isValid() { return socket->isValid(); }

void SyncClient::disconnectServer() {
  if (socket->isValid())
    socket->close();
  socket->disconnect();
}

int SyncClient::sendMessage(const QByteArray &message) {
  if (!socket->isValid())
    return 0;

  if (socket->isValid()) {
    auto bytesSent = socket->sendBinaryMessage(message);
    (void)bytesSent;
    assert(bytesSent == message.size() && "Client failed to send the message.");
    return 0;
  } else {
    return 1;
  }
}
