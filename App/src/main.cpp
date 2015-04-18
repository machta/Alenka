#include "error.h"
#include "myapplication.h"
#include "signalfilebrowserwindow.h"

#include <QLocale>

#include <stdexcept>
#include <sstream>

using namespace std;

#if CL_VERSION_1_1
#else
#error OpenCL 1.1 or later required.
#endif

int main(int argc, char** argv)
{
	try
	{
		// Set locale.
		QLocale locale("en_us");
		QLocale::setDefault(locale);

		// Set up the application.
		MyApplication app(argc, argv);

		SignalFileBrowserWindow window;
		window.show();

		return app.exec();
	}
	catch (std::exception& e)
	{
		logToFileAndConsole("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToFileAndConsole("Unknown exception caught.");
	}

	return EXIT_FAILURE;
}
