#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <sstream>
#include <iostream>
#include <stdexcept>

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
			if (functions == nullptr)
			{
				stringstream ss;
				ss << "versionFunctions<QOpenGLFunctions_4_1_Core>() failed.";
				throw runtime_error(ss.str());
			}

			bool res = functions->initializeOpenGLFunctions();
			if (res == false)
			{
				stringstream ss;
				ss << "initializeOpenGLFunctions() failed.";
				throw runtime_error(ss.str());
			}
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
			if (res == false)
			{
				stringstream ss;
				ss << "logger->initialize() failed.";
				throw runtime_error(ss.str());
			}
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
			stringstream ss;
			ss << "OpenGL error detected.";
			throw runtime_error(ss.str());
		}
	}
};

#endif // OPENGLINTERFACE_H
