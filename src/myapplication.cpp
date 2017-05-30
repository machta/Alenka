#include "myapplication.h"

#include "options.h"
#include "error.h"
#include <AlenkaSignal/openclcontext.h>

#include <QSurfaceFormat>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QDir>

#include <stdexcept>
#include <string>

using namespace std;

MyApplication::MyApplication(int& argc, char** argv) : QApplication(argc, argv)
{
	// Disable the stupid ssl warning.
	QLoggingCategory::setFilterRules("qt.network.ssl=false");

	try
	{
		// Set up the global options object.
		options = new Options(argc, argv);
		PROGRAM_OPTIONS_POINTER = SET_PROGRAM_OPTIONS_POINTER = options;

		// Set up the log.
		const int maxLogFileNameLength = 1000;
		char logFileName[maxLogFileNameLength + 1];
		time_t now = time(nullptr);
		size_t len = strftime(logFileName, maxLogFileNameLength, "%Y-%m-%d--%H-%M-%S.log", localtime(&now));
		logFileName[len] = 0;

		char s = dirSeparator();
		string logFilePath = applicationDirPath().toStdString() + s + "log" + s + logFileName;
		LOG_FILE.open(logFilePath);

		if (!LOG_FILE.good())
			LOG_FILE.open(logFileName);

		checkErrorCode(LOG_FILE.good(), true, "Could not open log file '" + string(logFilePath) + "' for writing.");
	}
	catch (exception& e)
	{
		QMessageBox::critical(nullptr, "Exception caught", QString::fromStdString(e.what()));
		mainExit(EXIT_FAILURE);
	}
	catch (...)
	{
		QMessageBox::critical(nullptr, "Exception caught", "Unknown exception caught.");
		mainExit(EXIT_FAILURE);
	}

#ifdef UNIX_BUILD
	logToFile("Alenka " << versionString() << " Unix build");
#endif

#ifdef WIN_BUILD
	logToFile("Alenka " << versionString() << " Windows build");
#endif

	// Log the command line parameters and config file.
	{
		stringstream ss;
		ss << "Starting with command: ";

		for (int i = 0; i < argc; ++i)
		{
			if (i != 0)
				ss << " ";

			ss << argv[i];
		}

		logToFile(ss.str());
	}

	PROGRAM_OPTIONS.logConfigFile();

	// Initialize the global OpenCL context.
	globalContext.reset(new AlenkaSignal::OpenCLContext(PROGRAM_OPTIONS["clPlatform"].as<int>(), PROGRAM_OPTIONS["clDevice"].as<int>()));

	// Set up the clFFT library.
	AlenkaSignal::OpenCLContext::clfftInit();

	// Set some OpenGL context details.
	QSurfaceFormat format = QSurfaceFormat::defaultFormat();

	format.setVersion(2, 0);
	format.setProfile(QSurfaceFormat::NoProfile);

#ifndef NDEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif

	QSurfaceFormat::setDefaultFormat(format);

	// Process some of the command-line only options.
	stringstream ss;

	ss << globalContext->getPlatformInfo() << endl << endl;
	ss << globalContext->getDeviceInfo() << endl;

	logToFile(ss.str());

	if (PROGRAM_OPTIONS.isSet("help"))
	{
		cout << "Usage:" << endl;
		cout << "  Alenka [OPTION]... [FILE]..." << endl;
		cout << "  Alenka --spikedet OUTPUT_FILE [SPIKEDET_SETTINGS]... FILE [FILE]..." << endl;
		cout << "  Alenka --help|--clInfo|--version" << endl;
		cout << PROGRAM_OPTIONS.getDescription() << endl;
		mainExit();
	}
	else if (PROGRAM_OPTIONS.isSet("clInfo"))
	{
		cout << ss.str();
		mainExit();
	}
	else if (PROGRAM_OPTIONS.isSet("version"))
	{
		cout << "Alenka " << versionString() << endl;
		mainExit();
	}

	// Set locale.
	QLocale locale(PROGRAM_OPTIONS["locale"].as<string>().c_str());
	QLocale::setDefault(locale);
}

MyApplication::~MyApplication()
{
	AlenkaSignal::OpenCLContext::clfftDeinit();

	delete options;
}

bool MyApplication::notify(QObject* receiver, QEvent* event)
{
	try
	{
		return QApplication::notify(receiver, event);
	}
	catch (exception& e)
	{
		logToFileAndConsole("Standard exception caught: " << e.what());
	}
	catch (...)
	{
		logToFileAndConsole("Unknown exception caught.");
	}

	mainExit(EXIT_FAILURE);
	return false;
}

void MyApplication::mainExit(int status)
{
	logToFile("Exiting with status " << status << ".");
	std::exit(status);
}

char MyApplication::dirSeparator()
{
	return QDir::separator().toLatin1();
}

unique_ptr<AlenkaSignal::OpenCLContext> globalContext(nullptr);
