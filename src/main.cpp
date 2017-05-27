/**
 * @brief Definition of main().
 *
 * Checks whether Windows or Unix build was specified (WIN_BUILD or UNIX_BUILD).
 *
 * @file
 */

#include "error.h"
#include "myapplication.h"
#include "signalfilebrowserwindow.h"
#include "options.h"
#include "spikedetanalysis.h"

#include <stdexcept>
#include <string>

using namespace std;

// Enforce definition of either WIN_BUILD or UNIX_BUILD.
#if defined WIN_BUILD
	#if defined UNIX_BUILD
		#error Only one of WIN_BUILD or UNIX_BUILD can be defined.
	#endif
#elif defined UNIX_BUILD
	#if defined WIN_BUILD
		#error Only one of WIN_BUILD or UNIX_BUILD can be defined.
	#endif
#else
	#error You must define one of WIN_BUILD or UNIX_BUILD.
#endif

int main(int argc, char** argv)
{
	int ret = EXIT_FAILURE;

	try
	{
		MyApplication app(argc, argv);

		if (PROGRAM_OPTIONS.isSet("spikedet"))
		{
			if (PROGRAM_OPTIONS.isSet("filename"))
			{
				SpikedetAnalysis::analyseCommandLineFile();
				MyApplication::mainExit();
			}
			else
			{
				cerr << "Error: no input file specified" << endl;
				MyApplication::mainExit(EXIT_FAILURE);
			}
		}

		SignalFileBrowserWindow window;

		if (PROGRAM_OPTIONS["mode"].as<string>() == "tablet" || PROGRAM_OPTIONS["mode"].as<string>() == "tablet-full")
			window.showMaximized();
		else
			window.show();

		window.openCommandLineFile();
		ret = app.exec();
	}
	catch (exception& e)
	{
		logToFileAndConsole("Standard exception caught: " << e.what());
	}
	catch (...)
	{
		logToFileAndConsole("Unknown exception caught.");
	}

	MyApplication::mainExit(ret);
}
