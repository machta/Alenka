#include "syncdialog.h"

#include <iostream>

#include "../DataModel/opendatafile.h"
#include "syncclient.h"
#include "syncserver.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

using namespace std;

namespace {

int64_t unpackMessage(const QByteArray &message) {
  int64_t result;
  assert(message.size() == (int)sizeof(result));

  char *rawResult = reinterpret_cast<char *>(&result);
  copy(message.data(), message.data() + sizeof(result), rawResult);

  return result;
}

QByteArray packMessage(const int64_t val) {
  return QByteArray(reinterpret_cast<const char *>(&val), sizeof(val));
}

} // namespace

SyncDialog::SyncDialog(QWidget *parent) : QDialog(parent) {
  server = make_unique<SyncServer>();
  client = make_unique<SyncClient>();

  connect(server.get(), SIGNAL(messageReceived(QByteArray)), this,
          SLOT(receiveSyncMessage(QByteArray)));
  connect(client.get(), SIGNAL(messageReceived(QByteArray)), this,
          SLOT(receiveSyncMessage(QByteArray)));

  auto c =
      connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)),
              this, SLOT(sendSyncMessage()));
  //  openFileConnections.push_back(c);

  setWindowTitle("Synchronization Manager");
  auto box = new QVBoxLayout();

  combo = new QComboBox();
  combo->insertItem(0, "Server");
  combo->insertItem(1, "Client");
  box->addWidget(combo);

  buildServerControls();
  buildClientControls();

  box->addWidget(serverControls);
  box->addWidget(clientControls);
  clientControls->hide();

  connect(combo, SIGNAL(activated(QString)), this,
          SLOT(activateControls(QString)));

  setLayout(box);
}

void SyncDialog::buildServerControls() {
  QFont font;
  font.setPointSize(18);

  auto box = new QVBoxLayout();
  auto grid = new QGridLayout();

  auto label = new QLabel("Network interface IPs of this device:");
  label->setToolTip("Use this to connect with the client over LAN");
  grid->addWidget(label, 0, 0);

  QString ipAddresses;
  int ipCount = 0;
  for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
    if (address.protocol() == QAbstractSocket::IPv4Protocol &&
        address != QHostAddress(QHostAddress::LocalHost)) {
      if (ipCount++ > 0)
        ipAddresses += '\n';
      ipAddresses += address.toString();
    }
  }
  auto ipLabel = new QLabel(ipAddresses);
  ipLabel->setText(ipAddresses);
  ipLabel->setFont(font);
  grid->addWidget(ipLabel, 1, 0);

  grid->addWidget(new QLabel("Port:"), 0, 1);
  serverPortEdit = new QLineEdit("1234");
  serverPortEdit->setFont(font);
  grid->addWidget(serverPortEdit, 1, 1, Qt::AlignTop);

  box->addLayout(grid);
  auto buttonBox = new QDialogButtonBox();

  launchButton = new QPushButton("Launch");
  buttonBox->addButton(launchButton, QDialogButtonBox::ActionRole);
  connect(launchButton, SIGNAL(clicked(bool)), this, SLOT(launchServer()));

  auto button = new QPushButton("Shut down");
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);
  connect(button, SIGNAL(clicked(bool)), this, SLOT(shutDownServer()));

  box->addWidget(buttonBox);

  auto status = new QFormLayout();
  font.setPointSize(15);
  serverStatus = new QLabel("Server Ready");
  serverStatus->setFont(font);
  status->addRow("Status: ", serverStatus);
  box->addLayout(status);

  serverControls = new QWidget();
  serverControls->setLayout(box);
}

void SyncDialog::buildClientControls() {
  QFont font;
  font.setPointSize(18);

  auto box = new QVBoxLayout();
  auto grid = new QGridLayout();

  QLabel *label = new QLabel("Server address or IP:");
  label->setToolTip("Examples: www.example.com, 10.0.0.10");
  grid->addWidget(label, 0, 0);
  auto hbox = new QHBoxLayout();
  label = new QLabel("ws://");
  label->setFont(font);
  hbox->addWidget(label);
  clientIpEdit = new QLineEdit("localhost");
  clientIpEdit->setFont(font);
  hbox->addWidget(clientIpEdit);
  grid->addLayout(hbox, 1, 0);

  grid->addWidget(new QLabel("Port:"), 0, 1);
  clientPortEdit = new QLineEdit("1234");
  clientPortEdit->setFont(font);
  grid->addWidget(clientPortEdit, 1, 1);

  box->addLayout(grid);
  auto buttonBox = new QDialogButtonBox();

  connectButton = new QPushButton("Connect");
  buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
  connect(connectButton, SIGNAL(clicked(bool)), this, SLOT(connectClient()));

  auto button = new QPushButton("Disconnect");
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);
  connect(button, SIGNAL(clicked(bool)), this, SLOT(disconnectClient()));
  connect(client.get(), &SyncClient::serverDisconnected, [this]() {
    changeEnableControls(true);

    clientStatus->setText("Client Disconnected");
    clientStatus->setStyleSheet("QLabel { color : red; }");
  });

  box->addWidget(buttonBox);

  auto status = new QFormLayout();
  font.setPointSize(15);
  clientStatus = new QLabel("Client Ready");
  clientStatus->setFont(font);
  status->addRow("Status: ", clientStatus);
  box->addLayout(status);

  clientControls = new QWidget();
  clientControls->setLayout(box);
}

void SyncDialog::activateControls(const QString &text) {
  if (text == "Server") {
    clientControls->hide();
    serverControls->show();
  } else {
    serverControls->hide();
    clientControls->show();
  }
}

// TODO: Make better error messages.
void SyncDialog::launchServer() {
  bool ok;
  int port = serverPortEdit->text().toInt(&ok);

  if (ok) {
    if (server->launch(port))
      ok = false;
  }

  if (ok) {
    changeEnableControls(false);

    serverStatus->setText("Server Running");
    serverStatus->setStyleSheet("QLabel { color : green; }");
  } else {
    QMessageBox::critical(this, "Error", "Server launch failed.");
  }
}

void SyncDialog::shutDownServer() {
  server->shutDown();

  changeEnableControls(true);

  serverStatus->setText("Server Ready");
  serverStatus->setStyleSheet("QLabel { color : black; }");
}

void SyncDialog::connectClient() {
  bool ok;
  int port = clientPortEdit->text().toInt(&ok);

  QUrl url("ws://" + clientIpEdit->text());
  int result = 1;

  if (ok && url.isValid())
    result = client->connectServer(url, port);

  if (result) {
    QMessageBox::critical(this, "Error", "Connection failed.");
  } else {
    changeEnableControls(false);

    clientStatus->setText("Client Connected");
    clientStatus->setStyleSheet("QLabel { color : green; }");
  }
}

void SyncDialog::disconnectClient() {
  client->disconnectServer();

  changeEnableControls(true);

  clientStatus->setText("Client Ready");
  clientStatus->setStyleSheet("QLabel { color : black; }");
}

void SyncDialog::changeEnableControls(bool enable) {
  combo->setEnabled(enable);

  serverPortEdit->setEnabled(enable);
  launchButton->setEnabled(enable);

  clientIpEdit->setEnabled(enable);
  clientPortEdit->setEnabled(enable);
  connectButton->setEnabled(enable);
}

/**
 * @brief Interprets and acts on the received sync message.
 *
 * Also saves the last received position so that we can use it in
 * sendSyncMessage() to break the deadlock.
 */
void SyncDialog::receiveSyncMessage(const QByteArray &message) {
  if (!file || !shouldSynchronize)
    return;

  const int position = static_cast<int>(unpackMessage(message));
#ifndef NDEBUG
  cerr << "Received position: " << position << endl;
#endif
  lastPositionReceived = position;
  OpenDataFile::infoTable.setPosition(position);
}

/**
 * @brief Creates the sync message and sends it to both the server and the
 * client.
 *
 * It's OK to send the message to both, because they should never be active
 * simultaneously. So one of them will always ignore it.
 *
 * There is a special case in which the message shouldn't be sent ot prevent
 * message deadlock.This comes about because the source of the signal that
 * trigers this method cannot be distinguished between user input and a received
 * sync message.
 *
 * So when the current position is very close (within a small fraction of a
 * second) to the last received position, you are most likely just repeating
 * what the server already has. So there is no need to send this information
 * back again.
 */
void SyncDialog::sendSyncMessage() {
  if (!file || !shouldSynchronize)
    return;

  const int position = OpenDataFile::infoTable.getPosition();
  const int epsilon = max<int>(2, file->file->getSamplingFrequency() / 100);

  // This condition is to break the message deadlock.
  // TODO: Check whether this is still needed after the switch to new position.
  const bool shouldNotSkip = position < (lastPositionReceived - epsilon) ||
                             position > (lastPositionReceived + epsilon);
  if (shouldNotSkip) {
#ifndef NDEBUG
    if (server->connectionCount() > 0 || client->isValid())
      cerr << "Sending position: " << position << " " << position << endl;
#endif
    const QByteArray message = packMessage(position);
    server->sendMessage(message);
    client->sendMessage(message);
  }
#ifndef NDEBUG
  else {
    cerr << "Message skipped: " << position << endl;
  }
#endif

  // Reset to the default value, so that the message is skipped at most once.
  lastPositionReceived = lastPositionReceivedDefault;
}
