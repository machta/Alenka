#include "options.h"
#include "testwindow.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <iostream>
#include <stdexcept>

using namespace std;

int main(int ac, char** av)
{
	try
	{
		// Create global options object.
		Options* options = new Options(ac, av);
		PROGRAM_OPTIONS = options;

		if (options->isSet("help"))
		{
			cout << options->getDescription() << endl;

			delete options;
			return 0;
		}

		// Set the OpenGL context details.
		QSurfaceFormat format = QSurfaceFormat::defaultFormat();
		format.setVersion(4, 1);
		format.setProfile(QSurfaceFormat::CoreProfile);
#ifndef NDEBUG
		format.setOption(QSurfaceFormat::DebugContext);
#endif

		QSurfaceFormat::setDefaultFormat(format);

		// Create application and show main window.
		QApplication app(ac, av);

		TestWindow window;
		window.show();

		int res = app.exec();

		delete options;
		return res;
	}
	catch (exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception caught." << endl;
	}

	return -1;
}
