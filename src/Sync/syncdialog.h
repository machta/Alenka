#ifndef SYNCDIALOG_H
#define SYNCDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QPushButton;
class SyncServer;
class SyncClient;

class SyncDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SyncDialog(SyncServer* server, SyncClient* client, QWidget* parent = nullptr);

signals:

public slots:

private:
	SyncServer* server;
	SyncClient* client;
	QComboBox* combo;
	QWidget* serverControls;
	QWidget* clientControls;
	QLineEdit* serverPortEdit;
	QLineEdit* clientPortEdit;
	QLineEdit* clientIpEdit;
	QPushButton* launchButton;
	QPushButton* connectButton;

	void buildServerControls();
	void buildClientControls();

private slots:
	void activateControls(const QString &text);
	void launchServer();
	void connectClient();
};

#endif // SYNCDIALOG_H
