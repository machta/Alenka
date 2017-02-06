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
	QLineEdit* clientPortEdit;
	QLineEdit* severPortEdit;
	QLineEdit* serverIpEdit;
	QPushButton* connectButton;
	QPushButton* launchButton;

	void buildServerControls();
	void buildClientControls();

private slots:
	void activateControls(const QString &text);
};

#endif // SYNCDIALOG_H
