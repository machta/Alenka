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

#ifdef TESTS
#include <gtest/gtest.h>
#endif

#include <stdexcept>

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
#ifdef TESTS
		testing::InitGoogleTest(&argc, argv);
#endif
		MyApplication app(argc, argv);

#ifdef TESTS
		if (PROGRAM_OPTIONS.isSet("test"))
		{
			return RUN_ALL_TESTS();
		}
#endif

		SignalFileBrowserWindow window;
		window.show();

		if (PROGRAM_OPTIONS["tablet"].as<bool>())
			window.showMaximized();

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
