#include "openglinterface.h"

void OpenGLInterface::checkGLErrors()
{
	using namespace std;

	GLenum err;
	bool errorDetected = false;

	while (err = functions->glGetError(), err != GL_NO_ERROR)
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
