#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

#include <array>
#include <memory>

class Options;
namespace AlenkaSignal {
class OpenCLContext;
}

/**
 * @brief This class initializes objects and libraries needed to run the
 * program.
 */
class MyApplication : public QApplication {
  Q_OBJECT

  Options *options = nullptr;

public:
  /**
   * @brief MyApplication constructor.
   *
   * Qt is initialized by the QApplication constructor.
   *
   * Log and Options are initialized here as well as clFFT.
   * Some OpenGL context details are specified here.
   *
   * Parameters from main are passed to the base class and to Options.
   *
   * Also some of the command line only options are processed here.
   */
  explicit MyApplication(int &argc, char **argv);
  ~MyApplication();

  /**
   * @brief This method allows for handling exceptions originating from event
   * handlers.
   *
   * Currently when an exception is caught, it is logged and then program
   * execution continues. It is not clear whether this causes problems.
   * If so, abort call should be added here (like it is done in the loader
   * thread).
   */
  virtual bool notify(QObject *receiver, QEvent *event) override;

  static void mainExit(int status = EXIT_SUCCESS);

  static std::array<int, 3> version() { return std::array<int, 3>{0, 9, 2}; }
  static std::string versionString(const std::array<int, 3> &v) {
    using namespace std;
    return to_string(get<0>(v)) + '.' + to_string(get<1>(v)) + '.' +
           to_string(get<2>(v));
  }
  static std::string versionString() { return versionString(version()); }

  static char dirSeparator();
};

extern std::unique_ptr<AlenkaSignal::OpenCLContext> globalContext;

#endif // MYAPPLICATION_H
