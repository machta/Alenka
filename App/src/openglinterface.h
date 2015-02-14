#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include "error.h"

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <iostream>

class OpenGLInterface
{
public:
	OpenGLInterface() {} // Initialization is done when the interface pointers are needed for the first time.

	~OpenGLInterface()
	{
		delete logger;
	}

protected:
	QOpenGLFunctions_4_1_Core* fun()
	{
		using namespace std;

		if (functions == nullptr)
		{
			functions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_1_Core>();
			checkNotErrorCode(functions, nullptr, "versionFunctions<QOpenGLFunctions_4_1_Core>() failed.");

			bool res = functions->initializeOpenGLFunctions();
			checkNotErrorCode(res, false, "initializeOpenGLFunctions() failed.");
		}

		checkGLErrors();

		return functions;
	}

	QOpenGLDebugLogger* log()
	{
		using namespace std;

		if (logger == nullptr)
		{
			logger = new QOpenGLDebugLogger();

			bool res = logger->initialize();
			checkNotErrorCode(res, false, "logger->initialize() failed.");

			logger->logMessage(QOpenGLDebugMessage::createApplicationMessage("OpenGL debug log initialized."));
		}

		checkGLErrors();

		return logger;
	}

private:
	QOpenGLFunctions_4_1_Core* functions = nullptr;
	QOpenGLDebugLogger* logger = nullptr;

	void checkGLErrors()
	{
		using namespace std;

		GLenum err;
		bool errorDetected = false;

		while (err = functions->glGetError(), err != GL_NO_ERROR)
		{
			errorDetected = true;

			cerr << "OpenGL error: " << err << "(0x" << hex << err << ")" << endl;
		}

		if (errorDetected)
		{
			throw runtime_error("OpenGL error detected.");
		}
	}
};

#endif // OPENGLINTERFACE_H
