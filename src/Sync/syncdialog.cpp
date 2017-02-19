#include "syncdialog.h"

#include "syncserver.h"
#include "syncclient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QUrl>
#include <QNetworkInterface>
#include <QFont>
#include <QGridLayout>
#include <QFormLayout>

SyncDialog::SyncDialog(SyncServer* server, SyncClient* client, QWidget* parent) : QDialog(parent),
	server(server), client(client)
{
	setWindowTitle("Synchronization Manager");
	QVBoxLayout* box = new QVBoxLayout();

	combo = new QComboBox();
	combo->insertItem(0, "Server");
	combo->insertItem(1, "Client");
	box->addWidget(combo);

	buildServerControls();
	buildClientControls();
	box->addWidget(serverControls);
	box->addWidget(clientControls);
	clientControls->hide();

	connect(combo, SIGNAL(activated(QString)), this, SLOT(activateControls(QString)));

	setLayout(box);
}

void SyncDialog::buildServerControls()
{
	QFont font;
	font.setPointSize(18);

	QVBoxLayout* box = new QVBoxLayout();
	QGridLayout* grid = new QGridLayout();

	QLabel* label = new QLabel("Network interface IPs of this device:");
	label->setToolTip("Use this to connect with the client over LAN");
	grid->addWidget(label, 0, 0);

	QString ipAddresses;
	int ipCount = 0;
	for (const QHostAddress& address : QNetworkInterface::allAddresses())
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
		{
			if (ipCount++ > 0)
				ipAddresses += '\n';
			 ipAddresses += address.toString();
		}
	}
	QLabel* ipLabel = new QLabel(ipAddresses);
	ipLabel->setText(ipAddresses);
	ipLabel->setFont(font);
	grid->addWidget(ipLabel, 1, 0);

	grid->addWidget(new QLabel("Port:"), 0, 1);
	serverPortEdit = new QLineEdit("1234");
	serverPortEdit->setFont(font);
	grid->addWidget(serverPortEdit, 1, 1, Qt::AlignTop);

	box->addLayout(grid);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	launchButton = new QPushButton("Launch");
	buttonBox->addButton(launchButton, QDialogButtonBox::ActionRole);
	connect(launchButton, SIGNAL(clicked(bool)), this, SLOT(launchServer()));

	QPushButton* button = new QPushButton("Shut down");
	buttonBox->addButton(button, QDialogButtonBox::ActionRole);
	connect(button, SIGNAL(clicked(bool)), this, SLOT(shutDownServer()));

	box->addWidget(buttonBox);

	QFormLayout* status = new QFormLayout();
	font.setPointSize(15);
	serverStatus = new QLabel("Server Ready");
	serverStatus->setFont(font);
	status->addRow("Status: ", serverStatus);
	box->addLayout(status);

	serverControls = new QWidget();
	serverControls->setLayout(box);
}

void SyncDialog::buildClientControls()
{
	QFont font;
	font.setPointSize(18);

	QVBoxLayout* box = new QVBoxLayout();
	QGridLayout* grid = new QGridLayout();

	QLabel* label = new QLabel("Server address or IP:");
	label->setToolTip("Web address e.g. ws://example.com, or 10.0.0.10 (Note that you must use the WebSocket protocol.)");
	grid->addWidget(label, 0, 0);
	clientIpEdit = new QLineEdit("ws://localhost");
	clientIpEdit->setFont(font);
	grid->addWidget(clientIpEdit, 1, 0);

	grid->addWidget(new QLabel("Port:"), 0, 1);
	clientPortEdit = new QLineEdit("1234");
	clientPortEdit->setFont(font);
	grid->addWidget(clientPortEdit, 1, 1);

	box->addLayout(grid);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	connectButton = new QPushButton("Connect");
	buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
	connect(connectButton, SIGNAL(clicked(bool)), this, SLOT(connectClient()));

	QPushButton* button = new QPushButton("Disconnect");
	buttonBox->addButton(button, QDialogButtonBox::ActionRole);
	connect(button, SIGNAL(clicked(bool)), this, SLOT(disconnectClient()));
	connect(client, &SyncClient::serverDisconnected, [this] () {
		changeEnableControls(true);

		clientStatus->setText("Client Disconnected");
		clientStatus->setStyleSheet("QLabel { color : red; }");
	});

	box->addWidget(buttonBox);

	QFormLayout* status = new QFormLayout();
	font.setPointSize(15);
	clientStatus = new QLabel("Client Ready");
	clientStatus->setFont(font);
	status->addRow("Status: ", clientStatus);
	box->addLayout(status);

	clientControls = new QWidget();
	clientControls->setLayout(box);
}

void SyncDialog::activateControls(const QString& text)
{
	if (text == "Server")
	{
		clientControls->hide();
		serverControls->show();
	}
	else
	{
		serverControls->hide();
		clientControls->show();
	}
}

//TODO: Make better error messages.
void SyncDialog::launchServer()
{
	bool ok;
	int port = serverPortEdit->text().toInt(&ok);

	if (ok)
	{
		if (server->launch(port))
			ok = false;
	}

	if (ok)
	{
		changeEnableControls(false);

		serverStatus->setText("Server Running");
		serverStatus->setStyleSheet("QLabel { color : green; }");
	}
	else
	{
		QMessageBox::critical(this, "Error", "Server launch failed.");
	}
}

void SyncDialog::shutDownServer()
{
	server->shutDown();

	changeEnableControls(true);

	serverStatus->setText("Server Ready");
	serverStatus->setStyleSheet("QLabel { color : black; }");
}

void SyncDialog::connectClient()
{
	bool ok;
	int port = clientPortEdit->text().toInt(&ok);

	QUrl url(clientIpEdit->text());
	int result = 1;

	if (ok && url.isValid())
	{
		result = client->connectServer(url, port);
	}

	if (result)
	{
		QMessageBox::critical(this, "Error", "Connection failed.");
	}
	else
	{
		changeEnableControls(false);

		clientStatus->setText("Client Connected");
		clientStatus->setStyleSheet("QLabel { color : green; }");
	}
}

void SyncDialog::disconnectClient()
{
	client->disconnectServer();

	changeEnableControls(true);

	clientStatus->setText("Client Ready");
	clientStatus->setStyleSheet("QLabel { color : black; }");
}

void SyncDialog::changeEnableControls(bool enable)
{
	combo->setEnabled(enable);

	serverPortEdit->setEnabled(enable);
	launchButton->setEnabled(enable);

	clientIpEdit->setEnabled(enable);
	clientPortEdit->setEnabled(enable);
	connectButton->setEnabled(enable);
}
