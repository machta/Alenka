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

SyncDialog::SyncDialog(SyncServer* server, SyncClient* client, QWidget* parent) : QDialog(parent), server(server), client(client)
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
	serverControls = new QWidget();
	QVBoxLayout* box = new QVBoxLayout();

	QHBoxLayout* hbox = new QHBoxLayout();
	QVBoxLayout* vbox = new QVBoxLayout();

	QLabel* label = new QLabel("IP of this device:");
	label->setToolTip("Use this to connect with the client");
	vbox->addWidget(label);

	QLineEdit* lineEdit = new QLineEdit("...");
	lineEdit->setEnabled(false);
	vbox->addWidget(lineEdit);

	hbox->addLayout(vbox);
	vbox = new QVBoxLayout();

	vbox->addWidget(new QLabel("Port:"));
	serverPortEdit = new QLineEdit("1234");
	vbox->addWidget(serverPortEdit);
	hbox->addLayout(vbox);

	box->addLayout(hbox);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	launchButton = new QPushButton("Launch");
	buttonBox->addButton(launchButton, QDialogButtonBox::ActionRole);
	connect(launchButton, SIGNAL(clicked(bool)), this, SLOT(launchServer()));

	QPushButton* button = new QPushButton("Shut down");
	buttonBox->addButton(button, QDialogButtonBox::ActionRole);

	box->addWidget(buttonBox);
	serverControls->setLayout(box);
}

void SyncDialog::buildClientControls()
{
	clientControls = new QWidget();
	QVBoxLayout* box = new QVBoxLayout();

	QHBoxLayout* hbox = new QHBoxLayout();
	QVBoxLayout* vbox = new QVBoxLayout();

	QLabel* label = new QLabel("Server address (IP):");
	label->setToolTip("Web address e.g. http://example.com, or 10.0.0.10");
	vbox->addWidget(label);
	clientIpEdit = new QLineEdit("ws://localhost");
	vbox->addWidget(clientIpEdit);

	hbox->addLayout(vbox);
	vbox = new QVBoxLayout();

	vbox->addWidget(new QLabel("Port:"));
	clientPortEdit = new QLineEdit("1234");
	vbox->addWidget(clientPortEdit);
	hbox->addLayout(vbox);

	box->addLayout(hbox);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	connectButton = new QPushButton("Connect");
	buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
	connect(connectButton, SIGNAL(clicked(bool)), this, SLOT(connectClient()));

	QPushButton* button = new QPushButton("Disconnect");
	buttonBox->addButton(button, QDialogButtonBox::ActionRole);

	box->addWidget(buttonBox);
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
		combo->setEnabled(false);
		serverPortEdit->setEnabled(false);
		launchButton->setEnabled(false);
	}
	else
	{
		QMessageBox::critical(this, "Error", "Server launch failed.");
	}
}

void SyncDialog::connectClient()
{
	bool ok;
	int port = clientPortEdit->text().toInt(&ok);

	QUrl url(clientIpEdit->text());
	int result = 1;

	if (ok && url.isValid())
	{
		result = client->connectToServer(url, port);
	}

	if (result)
	{
		QMessageBox::critical(this, "Error", "Connection failed.");
	}
	else
	{
		combo->setEnabled(false);
		clientIpEdit->setEnabled(false);
		clientPortEdit->setEnabled(false);
		connectButton->setEnabled(false);
	}

	/*int dummy = 0;
	for (int i = 0; i < 1000000000; i++)
		dummy++;
	client->sendMessage(QByteArray("bla bla"));*/
}
