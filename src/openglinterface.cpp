#include "openglinterface.h"

#include "error.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLContext>

using namespace std;

OpenGLInterface::~OpenGLInterface()
{
	delete logger;
}

void OpenGLInterface::initializeOpenGLInterface()
{
	logToFile("Initializing OpenGLInterface.");

	QOpenGLContext* c = QOpenGLContext::currentContext();
	checkErrorCode(c->isValid(), true, "is current context valid");

	functions = c->versionFunctions<QOpenGLFunctions_2_0>();
	checkNotErrorCode(functions, nullptr, "versionFunctions<QOpenGLFunctions_2_0>() failed.");

	checkNotErrorCode(functions->initializeOpenGLFunctions(), false, "initializeOpenGLFunctions() failed.");

	// Initialize log.
	logger = new QOpenGLDebugLogger();

	bool res = logger->initialize();
	checkNotErrorCode(res, false, "logger->initialize() failed.");

	logger->logMessage(QOpenGLDebugMessage::createApplicationMessage("OpenGL debug log initialized."));

	// Get extension function addresses.
	QFunctionPointer ptr;

	ptr = QOpenGLContext::currentContext()->getProcAddress("glGenVertexArrays");
	checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glGenVertexArrays\")");
	genVertexArrays = reinterpret_cast<void (*)(GLsizei, GLuint*)>(ptr);

	ptr = QOpenGLContext::currentContext()->getProcAddress("glDeleteVertexArrays");
	checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glDeleteVertexArrays\")");
	deleteVertexArrays = reinterpret_cast<void (*)(GLsizei, const GLuint*)>(ptr);

	ptr = QOpenGLContext::currentContext()->getProcAddress("glBindVertexArray");
	checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glBindVertexArray\")");
	bindVertexArray = reinterpret_cast<void (*)(GLuint)>(ptr);

	if (PROGRAM_OPTIONS["gl43"].as<bool>())
	{
		ptr = QOpenGLContext::currentContext()->getProcAddress("glBindVertexBuffer");
		checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glBindVertexBuffer\")");
		bindVertexBuffer = reinterpret_cast<void (*)(GLuint, GLuint, GLintptr, GLintptr)>(ptr);

		ptr = QOpenGLContext::currentContext()->getProcAddress("glVertexAttribFormat");
		checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glVertexAttribFormat\")");
		vertexAttribFormat = reinterpret_cast<void (*)(GLuint, GLint, GLenum, GLboolean, GLuint)>(ptr);

		ptr = QOpenGLContext::currentContext()->getProcAddress("glVertexAttribBinding");
		checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glVertexAttribBinding\")");
		vertexAttribBinding = reinterpret_cast<void (*)(GLuint, GLuint)>(ptr);

		ptr = QOpenGLContext::currentContext()->getProcAddress("glVertexBindingDivisor");
		checkNotErrorCode(ptr, nullptr, "getProcAddress(\"glVertexBindingDivisor\")");
		vertexBindingDivisor = reinterpret_cast<void (*)(GLuint, GLuint)>(ptr);
	}
}

void OpenGLInterface::checkGLErrors()
{
	GLenum err;
	int errorsDetected = 0;

	while (err = functions->glGetError(), err != GL_NO_ERROR && errorsDetected <= 10)
	{
		++errorsDetected;

		logToFileAndConsole("OpenGL error: " << getErrorCode(err) << " last call from " << lastCallFile << ":" << lastCallLine);
	}

	if (errorsDetected > 0)
		throw runtime_error(to_string(errorsDetected) + " OpenGL errors detected.");
}

#define CASE(a_) case a_: return #a_

string OpenGLInterface::getErrorCode(GLenum code)
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
