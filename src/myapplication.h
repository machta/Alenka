#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>
#include <QDir>

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

public:
  /**
   * @brief MyApplication constructor.
   *
   * Qt is initialized by this constructor.
   *
   * Log and Options are initialized here as well as clFFT.
   * Some OpenGL context details are specified here.
   *
   * Parameters from main are passed to the base class and to Options.
   *
   * Also some of the command line only options are processed here.
   */
  explicit MyApplication(int &argc, char **argv);
  ~MyApplication() override;

  /**
   * @brief This method allows for handling exceptions originating from event
   * handlers.
   *
   * Currently when an exception is caught, it is logged and then program
   * execution continues. It is not clear whether this causes problems.
   * If so, abort call should be added here.
   */
  bool notify(QObject *receiver, QEvent *event) override;

  static void mainExit(int status = EXIT_SUCCESS);
  static int logExitStatus(int status);

  static auto version() { return std::array<int, 3>{{0, 9, 10}}; }
  static std::string versionString(const std::array<int, 3> &v) {
    using namespace std;
    return to_string(get<0>(v)) + '.' + to_string(get<1>(v)) + '.' +
           to_string(get<2>(v));
  }
  static std::string versionString() { return versionString(version()); }

  static QDir makeSubdir(const QString &path,
                         const std::vector<QString> &relPath);
  static QDir makeAppSubdir(const std::vector<QString> &relPath) {
    return makeSubdir(QApplication::applicationDirPath(), relPath);
  }
};

extern std::unique_ptr<AlenkaSignal::OpenCLContext> globalContext;

#endif // MYAPPLICATION_H
