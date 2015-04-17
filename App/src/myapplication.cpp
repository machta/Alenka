#include "myapplication.h"

#include "options.h"
#include "error.h"
#include "openclcontext.h"

#include <QSurfaceFormat>
#include <clFFT.h>

#include <stdexcept>
#include <string>

using namespace std;

MyApplication::MyApplication(int& argc, char** argv) : QApplication(argc, argv)
{
	// Set up the clFFT library.
	clfftStatus errFFT;
	clfftSetupData setupData;

	errFFT = clfftInitSetupData(&setupData);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftInitSetupData()");

	errFFT = clfftSetup(&setupData);
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftSetup()");

	// Set up the global options object.
	options = new Options(argc, argv);
	PROGRAM_OPTIONS_POINTER = options;

	// Set up the log.
	const int maxLogFileNameLength = 1000;
	char logFileName[maxLogFileNameLength + 1];
	time_t now = time(nullptr);
	size_t len = strftime(logFileName, maxLogFileNameLength, PROGRAM_OPTIONS["logFileName"].as<string>().c_str(), localtime(&now));
	logFileName[len] = 0;

	LOG_FILE.open(logFileName);
	checkErrorCode(LOG_FILE.good(), true, "Could not open log file for writing.");

	// Set some OpenGL context details.
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

		std::exit(EXIT_SUCCESS);
	}
	else if (PROGRAM_OPTIONS.isSet("clInfo"))
	{
		OpenCLContext context(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS);

		cout << "Platform" << endl;
		cout << context.getPlatformInfo() << endl << endl;
		cout << "Device" << endl;
		cout << context.getDeviceInfo() << endl;

		std::exit(EXIT_SUCCESS);
	}
}

MyApplication::~MyApplication()
{
	delete options;

	clfftStatus errFFT = clfftTeardown();
	checkErrorCode(errFFT, CLFFT_SUCCESS, "clfftTeardown()");
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

	return false; // TODO: possibly abort the application
}
