#include "myapplication.h"

#include "error.h"

#include <stdexcept>

MyApplication::MyApplication(int& argc, char** argv) : QApplication(argc, argv)
{
}

MyApplication::~MyApplication()
{
}

bool MyApplication::notify(QObject* receiver, QEvent* event)
{
	try
	{
		return QApplication::notify(receiver, event);
	}
	catch (std::exception& e)
	{
		logToBoth("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToBoth("Unknown exception caught.");
	}

	return false;// TODO: possibly abort the application
}
