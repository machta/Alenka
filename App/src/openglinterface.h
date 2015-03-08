#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include "error.h"

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <iostream>

class OpenGLInterface
{
public:
	//OpenGLInterface() {}
	// Initialization is done when the interface pointers are needed for the first time. Initialize() could be added instead so that the condition can be removed from fun() and log().

	~OpenGLInterface()
	{
		delete logger;
	}

protected:
#ifndef NDEBUG
	QOpenGLFunctions_4_1_Core* gl(const char* file = "", int line = 0)
#else
	QOpenGLFunctions_4_1_Core* gl(const char*, int)
#endif
	{
		using namespace std;

		if (functions == nullptr)
		{
			QOpenGLContext* c = QOpenGLContext::currentContext();
			checkErrorCode(c->isValid(), true, "is current context valid");

			functions = c->versionFunctions<QOpenGLFunctions_4_1_Core>();
			checkNotErrorCode(functions, nullptr, "versionFunctions<QOpenGLFunctions_4_1_Core>() failed.");

			checkNotErrorCode(functions->initializeOpenGLFunctions(), false, "initializeOpenGLFunctions() failed.");
		}

		checkGLErrors();

#ifndef NDEBUG
		lastCallFile = file;
		lastCallLine = line;
#endif

		return functions;
	}

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
	QOpenGLFunctions_4_1_Core* functions = nullptr;
	QOpenGLDebugLogger* logger = nullptr;

#ifndef NDEBUG
	const char* lastCallFile = "none";
	int lastCallLine = -1;
#endif

	void checkGLErrors()
	{
		using namespace std;

		GLenum err;
		bool errorDetected = false;

		while (err = functions->glGetError(), err != GL_NO_ERROR)
		{
			errorDetected = true;

			cerr << "OpenGL error: " << err << "(0x" << hex << err << dec << ")";
#ifndef NDEBUG
			cerr << " last call from " << lastCallFile << ":" << lastCallLine;
#endif
			cerr << endl;
		}

		if (errorDetected)
		{
			throw runtime_error("OpenGL error detected.");
		}
	}
};

#define gl() gl(__FILE__, __LINE__)

#endif // OPENGLINTERFACE_H
