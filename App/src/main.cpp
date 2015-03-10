#include "options.h"
#include "testwindow.h"
#include "openclcontext.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <clFFT.h>

#include <iostream>
#include <stdexcept>

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
			// Create application and show main window.
			QApplication app(ac, av);

			TestWindow window;
			window.show();

			ret = app.exec();
		}

		// Clean up.
		clfftTeardown();
		delete options;
	}
	catch (exception& e)
	{
		cerr << "Exception caught in main(): " << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception caught in main()." << endl;
	}

	return ret;
}
