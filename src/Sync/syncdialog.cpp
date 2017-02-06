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
	clientPortEdit = new QLineEdit("80");
	vbox->addWidget(clientPortEdit);
	hbox->addLayout(vbox);

	box->addLayout(hbox);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	launchButton = new QPushButton("Launch");
	buttonBox->addButton(launchButton, QDialogButtonBox::ActionRole);

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
	serverIpEdit = new QLineEdit("127.0.0.1");
	vbox->addWidget(serverIpEdit);

	hbox->addLayout(vbox);
	vbox = new QVBoxLayout();

	vbox->addWidget(new QLabel("Port:"));
	severPortEdit = new QLineEdit("80");
	vbox->addWidget(severPortEdit);
	hbox->addLayout(vbox);

	box->addLayout(hbox);
	QDialogButtonBox* buttonBox = new QDialogButtonBox();

	connectButton = new QPushButton("Connect");
	buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);

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
