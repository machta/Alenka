#include "options.h"
#include "testwindow.h"
#include "openclcontext.h"
#include "error.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <clFFT.h>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions/formatters/named_scope.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

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

		boost::log::add_file_log
		(
			boost::log::keywords::file_name = logFileName,
			boost::log::keywords::format =
			(
				boost::log::expressions::stream
				<< "["
				<< boost::log::expressions::attr<boost::log::attributes::local_clock::value_type>("TimeStamp")
				<< " "
				<< boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
				<< "] "
				<< boost::log::expressions::smessage
				<< " "
				<< boost::log::expressions::format_named_scope("Scope", boost::log::keywords::format = "[in %c():%F:%l]")
			)
		);
		boost::log::add_common_attributes();
		boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());

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
		logToBoth("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToBoth("Unknown exception caught.");
	}

	return ret;
}
