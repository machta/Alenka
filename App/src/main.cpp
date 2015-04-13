#include "options.h"
#include "signalfilebrowserwindow.h"
#include "openclcontext.h"
#include "error.h"
#include "myapplication.h"

#include <QLocale>
#include <QSurfaceFormat>
#include <clFFT.h>

#include <iostream>
#include <stdexcept>
#include <ctime>

using namespace std;

#if CL_VERSION_1_1
#else
#error OpenCL 1.1 or later required.
#endif

int main(int ac, char** av)
{
	int ret = EXIT_FAILURE;

	try
	{
		// Create global options object.
		Options* options = new Options(ac, av);
		PROGRAM_OPTIONS_POINTER = options;

		// Create log.
		char logFileName[500 + 1];
		time_t now = time(nullptr);
		size_t len = strftime(logFileName, 500, PROGRAM_OPTIONS["logFileName"].as<string>().c_str(), localtime(&now));
		logFileName[len] = 0;

		LOG_FILE.open(logFileName);
		checkErrorCode(LOG_FILE.good(), true, "Could not open log file for writing.");

		// Set up the clFFT library.
		clfftSetupData setupData;

		clfftStatus errFFT = clfftInitSetupData(&setupData);
		checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftInitSetupData()");

		errFFT = clfftSetup(&setupData);
		checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftSetup()");

		// Set the OpenGL context details.
		QSurfaceFormat format = QSurfaceFormat::defaultFormat();
		format.setVersion(4, 1);
		format.setProfile(QSurfaceFormat::CoreProfile);
#ifndef NDEBUG
		format.setOption(QSurfaceFormat::DebugContext);
#endif

		QSurfaceFormat::setDefaultFormat(format);

		// Process some of the commandline only options.
		if (PROGRAM_OPTIONS.isSet("help"))
		{
			cout << PROGRAM_OPTIONS.getDescription() << endl;

			ret = EXIT_SUCCESS;
		}
		else if (PROGRAM_OPTIONS.isSet("clInfo"))
		{
			OpenCLContext context(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS);

			cout << "Platform" << endl;
			cout << context.getPlatformInfo() << endl << endl;
			cout << "Device" << endl;
			cout << context.getDeviceInfo() << endl;

			ret = EXIT_SUCCESS;
		}
		else
		{
			// Set locale;
			QLocale locale("en_us");
			QLocale::setDefault(locale);

			// Create application and show main window.
			MyApplication app(ac, av);

			SignalFileBrowserWindow window;
			window.show();

			ret = app.exec();
		}

		// Clean up.
		clfftTeardown();
		delete options;
	}
	catch (exception& e)
	{
		logToBoth("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToBoth("Unknown exception caught.");
	}

	return ret;
}
