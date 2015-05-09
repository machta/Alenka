/**
 * @brief Definition of main() is here.
 *
 * In this source file is a compile time check of OpenCL 1.1 availability.
 *
 * @file
 */

#include "error.h"
#include "myapplication.h"
#include "signalfilebrowserwindow.h"

#include <stdexcept>

using namespace std;

#if CL_VERSION_1_1
#else
#error OpenCL 1.1 or later required.
#endif

int main(int argc, char** argv)
{
	int ret = EXIT_FAILURE;

	try
	{
		MyApplication app(argc, argv);

		SignalFileBrowserWindow window;
		window.show();

		ret = app.exec();
	}
	catch (std::exception& e)
	{
		logToFileAndConsole("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToFileAndConsole("Unknown exception caught.");
	}

	logToFile("Exiting with error code " << ret << ".");
	return ret;
}
