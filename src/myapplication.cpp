#include "myapplication.h"

#include "../Alenka-Signal/include/AlenkaSignal/openclcontext.h"
#include "error.h"
#include "options.h"

#include <QDir>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QSurfaceFormat>

#include <stdexcept>
#include <string>

using namespace std;

MyApplication::MyApplication(int &argc, char **argv)
    : QApplication(argc, argv) {
  // Disable the stupid ssl warning.
  QLoggingCategory::setFilterRules("qt.network.ssl=false");

  try {
    // Set up the global options object.
    PROGRAM_OPTIONS = make_unique<Options>(argc, argv);

    // Set up the log. This is a potential bug. TODO: Do it in a way doesn't
    // rely on a max length.
    const int maxLogFileNameLength = 1000;
    char logFileName[maxLogFileNameLength + 1];
    const time_t now = time(nullptr);
    const size_t len = strftime(logFileName, maxLogFileNameLength,
                                "%Y-%m-%d--%H-%M-%S.log", localtime(&now));
    logFileName[len] = 0;

    string logFilePath =
        makeAppSubdir({"log", logFileName}).absolutePath().toStdString();
    LOG_FILE.open(logFilePath);

    if (!LOG_FILE.good())
      LOG_FILE.open(logFileName);

    checkErrorCode(LOG_FILE.good(), true,
                   "Could not open log file '" << logFilePath
                                               << "' for writing.");
  } catch (const exception &e) {
    QMessageBox::critical(nullptr, "Exception caught",
                          QString::fromStdString(catchDetailed(e)));
    mainExit(EXIT_FAILURE);
  } catch (...) {
    QMessageBox::critical(nullptr, "Exception caught",
                          "Unknown exception caught.");
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

    for (int i = 0; i < argc; ++i) {
      if (i != 0)
        ss << " ";

      ss << argv[i];
    }

    logToFile(ss.str());
  }

  PROGRAM_OPTIONS->logConfigFile();

  // Process some of the command-line-only options.
  const unsigned int platformIndex = programOption<int>("clPlatform");
  const unsigned int deviceIndex = programOption<int>("clDevice");
  const string clInfoString =
      AlenkaSignal::OpenCLContext::getPlatformInfo(platformIndex) + "\n\n" +
      AlenkaSignal::OpenCLContext::getDeviceInfo(platformIndex, deviceIndex) +
      "\n";
  logToFile(clInfoString);

  if (isProgramOptionSet("help")) { // Help should be always the first.
    cout << R"(Usage:
  Alenka [OPTION]... [FILE]...
  Alenka --spikedet OUTPUT_FILE [SPIKEDET_SETTINGS]... FILE [FILE]...
  Alenka --help|--clInfo|--glInfo|--version
)";
    cout << PROGRAM_OPTIONS->getDescription();
    cout << "\nYou can find more details about these options in 'options.ini' "
            "or the manual."
         << endl;
    mainExit();
  } else if (isProgramOptionSet("version")) {
    cout << "Alenka " << versionString() << endl;
    mainExit();
  } else if (isProgramOptionSet("clInfo")) {
    cout << clInfoString;
    mainExit();
  }

  // Initialize the global OpenCL context.
  globalContext =
      make_unique<AlenkaSignal::OpenCLContext>(platformIndex, deviceIndex);

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

  // Set locale.
  QLocale locale(programOption<string>("locale").c_str());
  QLocale::setDefault(locale);
}

MyApplication::~MyApplication() { AlenkaSignal::OpenCLContext::clfftDeinit(); }

bool MyApplication::notify(QObject *receiver, QEvent *event) {
  try {
    return QApplication::notify(receiver, event);
  } catch (const exception &e) {
    logToFileAndConsole("Standard exception caught: " << catchDetailed(e));
  } catch (...) {
    logToFileAndConsole("Unknown exception caught.");
  }

  mainExit(EXIT_FAILURE);
  return false;
}

void MyApplication::mainExit(int status) {
  logExitStatus(status);
  std::exit(status);
}

int MyApplication::logExitStatus(int status) {
  logToFile("Exiting with status " << status << ".");
  return status;
}

QDir MyApplication::makeSubdir(const QString &path,
                               const std::vector<QString> &relPath) {
  QString dirPath = path;

  for (auto &e : relPath) {
    dirPath += QDir::separator() + e;
  }

  return QDir(dirPath);
}

unique_ptr<AlenkaSignal::OpenCLContext> globalContext(nullptr);
