#include "openglinterface.h"

#include "error.h"

#include <QOpenGLContext>

using namespace std;

void OpenGLInterface::initializeOpenGLInterface()
{
	logToFile("Initializing OpenGLInterface.");

	// initialization for gl()
	QOpenGLContext* c = QOpenGLContext::currentContext();
	checkErrorCode(c->isValid(), true, "is current context valid");

	functions = c->versionFunctions<QOpenGLFunctions_type>();
	checkNotErrorCode(functions, nullptr, "versionFunctions<>() failed.");

	checkNotErrorCode(functions->initializeOpenGLFunctions(), false, "initializeOpenGLFunctions() failed.");

	// initialization for log()
	logger = new QOpenGLDebugLogger();

	checkNotErrorCode(logger->initialize(), false, "logger->initialize() failed.");

	logger->logMessage(QOpenGLDebugMessage::createApplicationMessage("OpenGL debug log initialized."));

	// get extension function addresses
#if defined GL_2_0
	auto ptr1 = reinterpret_cast<void (*)(GLsizei, GLuint*)>(QOpenGLContext::currentContext()->getProcAddress("glGenVertexArrays"));
	checkNotErrorCode(ptr1, nullptr, "getProcAddress(\"glGenVertexArrays\")");
	genVertexArrays = ptr1;

	auto ptr2 = reinterpret_cast<void (*)(GLsizei, const GLuint*)>(QOpenGLContext::currentContext()->getProcAddress("glDeleteVertexArrays"));
	checkNotErrorCode(ptr2, nullptr, "getProcAddress(\"glDeleteVertexArrays\")");
	deleteVertexArrays = ptr2;

	auto ptr3 = reinterpret_cast<void (*)(GLuint)>(QOpenGLContext::currentContext()->getProcAddress("glBindVertexArray"));
	checkNotErrorCode(ptr3, nullptr, "getProcAddress(\"glBindVertexArray\")");
	bindVertexArray = ptr3;
#endif
}

void OpenGLInterface::checkGLErrors()
{
	using namespace std;

	GLenum err;
	bool errorDetected = false;
	int maxIterations = 10*1000; // This is to prevent infinite loop and filling the hard drive with garbage.

	while (err = functions->glGetError(), err != GL_NO_ERROR && maxIterations-- > 0)
	{
		errorDetected = true;

#ifdef NDEBUG
		logToFileAndConsole("OpenGL error: " << getErrorCode(err));
#else
		logToFileAndConsole("OpenGL error: " << getErrorCode(err) << " last call from " << lastCallFile << ":" << lastCallLine);
#endif
	}

	if (errorDetected)
	{
		throw runtime_error("OpenGL error detected.");
	}
}

#define CASE(a_) case a_: return #a_

std::string OpenGLInterface::getErrorCode(GLenum code)
{
	switch (code)
	{
		CASE(GL_INVALID_ENUM);
		CASE(GL_INVALID_VALUE);
		CASE(GL_INVALID_OPERATION);
		CASE(GL_STACK_OVERFLOW);
		CASE(GL_STACK_UNDERFLOW);
		CASE(GL_OUT_OF_MEMORY);
	}

	return "unknown code " + errorCodeToString(code);
}

#undef CASE
