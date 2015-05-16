/**
 * @brief The header defining the OpenGLInterface class.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include "error.h"

#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLDebugLogger>

#include <cassert>
#include <functional>

/**
 * @brief This class provides extending classes with the interface needed to call OpenGL API.
 *
 * Before any of the protected functions are used initializeOpenGLInterface()
 * must be called.
 *
 * All functions must be called from within an active OpenGL context section
 * (i.e. between makeCurrent(); and doneCurrent();).
 *
 */
class OpenGLInterface
{
#if defined GL_2_0
	using QOpenGLFunctions_type = QOpenGLFunctions_2_0;
#elif defined GL_3_0
	using QOpenGLFunctions_type = QOpenGLFunctions_3_0;
#elif defined GL_3_2
	using QOpenGLFunctions_type = QOpenGLFunctions_3_2_Core;
#endif

public:
	~OpenGLInterface()
	{
		delete logger;
	}

protected:
	void initializeOpenGLInterface();

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
	QOpenGLFunctions_type* gl(const char* file = "", int line = 0)
#else
	QOpenGLFunctions_type* gl(const char*, int)
#endif
	{
		assert(functions != nullptr);

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
		assert(logger != nullptr);

		checkGLErrors();

		return logger;
	}

	void glGenVertexArrays(GLsizei n, GLuint* arrays)
	{
#if defined GL_2_0
		checkGLErrors();
		genVertexArrays(n, arrays);
#ifndef NDEBUG
		lastCallFile = __FILE__;
		lastCallLine = __LINE__;
#endif
#else
		gl()->glGenVertexArrays(n, arrays);
#endif
	}

	void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
	{
#if defined GL_2_0
		checkGLErrors();
		deleteVertexArrays(n, arrays);
#ifndef NDEBUG
		lastCallFile = __FILE__;
		lastCallLine = __LINE__;
#endif
#else
		gl()->glDeleteVertexArrays(n, arrays);
#endif
	}

	void glBindVertexArray(GLuint array)
	{
#if defined GL_2_0
		checkGLErrors();
		bindVertexArray(array);
#ifndef NDEBUG
		lastCallFile = __FILE__;
		lastCallLine = __LINE__;
#endif
#else
		gl()->glBindVertexArray(array);
#endif
	}

private:
	QOpenGLFunctions_type* functions = nullptr;
	QOpenGLDebugLogger* logger = nullptr;
	std::function<void (GLsizei, GLuint*)> genVertexArrays;
	std::function<void (GLsizei, const GLuint*)> deleteVertexArrays;
	std::function<void (GLuint)> bindVertexArray;

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

/**
 * @brief Macro that passes some debug info to the gl() method.
 */
#define gl() gl(__FILE__, __LINE__)

#endif // OPENGLINTERFACE_H
