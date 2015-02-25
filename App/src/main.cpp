#include "options.h"
#include "testwindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <clFFT.h>

#include <iostream>
#include <stdexcept>

using namespace std;

int main(int ac, char** av)
{
	try
	{
		// Create global options object.
		Options* options = new Options(ac, av);
		PROGRAM_OPTIONS_POINTER = options;

		if (options->isSet("help"))
		{
			cout << options->getDescription() << endl;

			delete options;
			return 0;
		}

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

		// Create application and show main window.
		QApplication app(ac, av);

		TestWindow window;
		window.show();

		int res = app.exec();

		// Clean up.
		clfftTeardown();

		delete options;
		return res;
	}
	catch (exception& e)
	{
		cerr << "Exception caught in main(): " << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception caught in main()." << endl;
	}

	return EXIT_FAILURE;
}
