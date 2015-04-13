#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

class MyApplication : public QApplication
{
	Q_OBJECT

public:
	explicit MyApplication(int& argc, char** argv);
	~MyApplication();

	virtual bool notify(QObject* receiver, QEvent* event) override;
};

#endif // MYAPPLICATION_H
