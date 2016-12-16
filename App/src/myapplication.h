#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

#include <memory>

class Options;
namespace AlenkaSignal
{
class OpenCLContext;
}

/**
 * @brief This class initializes objects and libraries needed to run the program.
 */
class MyApplication : public QApplication
{
	Q_OBJECT

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
	explicit MyApplication(int& argc, char** argv);

	~MyApplication();

	/**
	 * @brief This method allows for handling exceptions originating from event handlers.
	 *
	 * Currently when an exception is caught, it is logged and then program
	 * execution continues. It is not clear whether this causes problems.
	 * If so, abort call should be added here (like it is done in the loader thread).
	 */
	virtual bool notify(QObject* receiver, QEvent* event) override;

private:
	Options* options = nullptr;
};

extern std::unique_ptr<AlenkaSignal::OpenCLContext> globalContext;

#endif // MYAPPLICATION_H
