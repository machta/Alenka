/**
 * @brief Definition of main().
 *
 * Compile time checks of program-wide macros are done here:
 *
 * * check whether a supported version of OpenGL (GL_2_0, GL_3_0 or GL_3_2) and
 * * OpenCL was selected (CL_1_1 or CL_1_2),
 * * check whether Windows or Unix build was specified (WIN_BUILD or UNIX_BUILD).
 *
 * Conflicting definitions are not allowed and result in early errors
 * to prevent potential problems.
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

// Enforce proper definition of a CL_ macro.
#if defined CL_1_1
	#if defined CL_1_2
		#error Only one of CL_1_1 or CL_1_2 can be defined.
	#endif

	#if !CL_VERSION_1_1
		#error OpenCL 1.1 is required.
	#endif
#elif defined CL_1_2
	#if defined GL_1_1
		#error Only one of CL_1_1 or CL_1_2 can be defined.
	#endif

	#if !CL_VERSION_1_2
		#error OpenCL 1.2 is required.
	#endif
#else
	#error You must define one of CL_1_1 or CL_1_2 to select the OpenCL version to be used.
#endif

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

		ret = app.exec();
	}
	catch (exception& e)
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
