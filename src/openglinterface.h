/**
 * @brief The header defining the OpenGLInterface class.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_2_0>

#include <cassert>
#include <functional>

class QOpenGLDebugLogger;

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
	~OpenGLInterface();

protected:
	/**
	 * @brief Delayed initialization function.
	 */
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
	QOpenGLFunctions_type* gl(const char* file, int line)
	{
		assert(functions);

		checkGLErrors();

		lastCallFile = file;
		lastCallLine = line;

		return functions;
	}

	/**
	 * @brief Macro that passes debug info to the gl().
	 */
	#define gl() gl(__FILE__, __LINE__)

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
		assert(logger);
		checkGLErrors();
		return logger;
	}

	void glGenVertexArrays(GLsizei n, GLuint* arrays)
	{
#if defined GL_2_0
		gl(); genVertexArrays(n, arrays);
#else
		gl()->glGenVertexArrays(n, arrays);
#endif
	}

	void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
	{
#if defined GL_2_0
		gl(); deleteVertexArrays(n, arrays);
#else
		gl()->glDeleteVertexArrays(n, arrays);
#endif
	}

	void glBindVertexArray(GLuint array)
	{
#if defined GL_2_0
		gl(); bindVertexArray(array);
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

	const char* lastCallFile = "";
	int lastCallLine = -1;

	/**
	 * @brief Checks if there are any OpenGL errors.
	 *
	 * If there are, print a message to the log and throw an exception.
	 *
	 * At most 10 messages are printed so that the output and log doesn't get flooded.
	 * This can happen on some platforms, where (for some reason) glGetError() never stops returning errors.
	 * In extreme cases this can lead to filling of the whole harddrive with the same error message.
	 */
	void checkGLErrors();

	/**
	 * @brief Converts enum item to a string name.
	 */
	std::string getErrorCode(GLenum code);
};

#endif // OPENGLINTERFACE_H
