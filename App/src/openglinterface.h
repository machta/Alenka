/**
 * @brief The header defining the OpenGLInterface class.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include "error.h"

#include <QOpenGLFunctions_3_0>
#include <QOpenGLDebugLogger>

#define GL_FUNCTIONS QOpenGLFunctions_3_0

/**
 * @brief This class provides extending classes with the interface needed to call OpenGL API.
 *
 * Initialization is done when the interface pointers are needed for the first time.
 * initialize() method could be added instead so that gl() and log() don't have
 * to check the pointer every time they are called.
 */
class OpenGLInterface
{
public:
	~OpenGLInterface()
	{
		delete logger;
	}

protected:
	/**
	 * @brief Returns a pointer needed for accessing the OpenGL API.
	 *
	 * If an error occurs it will be picked up by the next call to gl().
	 *
	 * In debug mode the file and line number of the last is stored.
	 * It is used to report from where the detected error originated
	 * (i.e. the position of the last call to gl()).
	 * In release mode this information is ignored.
	 */
#ifndef NDEBUG
	GL_FUNCTIONS* gl(const char* file = "", int line = 0)
#else
	GL_FUNCTIONS* gl(const char*, int)
#endif
	{
		using namespace std;

		if (functions == nullptr)
		{
			QOpenGLContext* c = QOpenGLContext::currentContext();
			checkErrorCode(c->isValid(), true, "is current context valid");

			functions = c->versionFunctions<GL_FUNCTIONS>();
			checkNotErrorCode(functions, nullptr, "versionFunctions<>() failed.");

			checkNotErrorCode(functions->initializeOpenGLFunctions(), false, "initializeOpenGLFunctions() failed.");
		}

		checkGLErrors();

#ifndef NDEBUG
		lastCallFile = file;
		lastCallLine = line;
#endif

		return functions;
	}

	/**
	 * @brief Returns a pointer needed to access the OpenGL debug log.
	 *
	 * This also checks OpenGL errors.
	 *
	 * For some reason the log is always empty.
	 * (I guess the OpenGL implementation doesn't use it or perhaps I didn't initialize it properly...)
	 */
	QOpenGLDebugLogger* log()
	{
		using namespace std;

		if (logger == nullptr)
		{
			logger = new QOpenGLDebugLogger();

			checkNotErrorCode(logger->initialize(), false, "logger->initialize() failed.");

			logger->logMessage(QOpenGLDebugMessage::createApplicationMessage("OpenGL debug log initialized."));
		}

		checkGLErrors();

		return logger;
	}

private:
	GL_FUNCTIONS* functions = nullptr;
	QOpenGLDebugLogger* logger = nullptr;

#ifndef NDEBUG
	const char* lastCallFile = "none";
	int lastCallLine = -1;
#endif

	/**
	 * @brief Checks if there are any OpenGL errors.
	 *
	 * If there are, print a message to the log and throw an exception.
	 */
	void checkGLErrors();

	/**
	 * @brief Converts enum item to a string name.
	 */
	std::string getErrorCode(GLenum code);
};

#undef GL_FUNCTIONS

/**
 * @brief Macro that passes some debug info to the gl() method.
 */
#define gl() gl(__FILE__, __LINE__)

#endif // OPENGLINTERFACE_H
