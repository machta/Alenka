/**
 * @brief Definition of main().
 *
 * Checks whether Windows or Unix build was specified (WIN_BUILD or UNIX_BUILD).
 *
 * @file
 */

#include "SignalProcessor/spikedetanalysis.h"
#include "error.h"
#include "myapplication.h"
#include "options.h"
#include "signalfilebrowserwindow.h"
#include <detailedexception.h>

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

int main(int argc, char **argv) {
  int ret = EXIT_FAILURE;

  try {
    MyApplication app(argc, argv);

    if (isProgramOptionSet("spikedet")) {
      if (isProgramOptionSet("filename")) {
        SpikedetAnalysis::analyseCommandLineFile();
        return MyApplication::logExitStatus(EXIT_SUCCESS);
      } else {
        cerr << "Error: no input file specified" << endl;
        return MyApplication::logExitStatus(EXIT_FAILURE);
      }
    }

    SignalFileBrowserWindow window;

    string mode;
    programOption("mode", mode);
    if (mode == "tablet" || mode == "tablet-full")
      window.showMaximized();
    else
      window.show();

    window.openCommandLineFile();
    ret = app.exec();
  } catch (const exception &e) {
    logToFileAndConsole("Standard exception caught: " << catchDetailed(e));
  } catch (...) {
    logToFileAndConsole("Unknown exception caught.");
  }

  return MyApplication::logExitStatus(ret);
}
