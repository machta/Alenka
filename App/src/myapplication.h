#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

class Options;

class MyApplication : public QApplication
{
	Q_OBJECT

public:
	explicit MyApplication(int& argc, char** argv);
	~MyApplication();

	virtual bool notify(QObject* receiver, QEvent* event) override;

private:
	Options* options = nullptr;
};

#endif // MYAPPLICATION_H
