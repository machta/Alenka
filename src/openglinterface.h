/**
 * @brief The header defining the OpenGLInterface class.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_4_3_Core>

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
	QOpenGLFunctions_2_0* functions20 = nullptr;
	QOpenGLFunctions_3_0* functions30 = nullptr;
	QOpenGLFunctions_4_3_Core* functions43 = nullptr;
	QOpenGLDebugLogger* logger = nullptr;

	std::function<void (GLsizei, GLuint*)> genVertexArrays = nullptr;
	std::function<void (GLsizei, const GLuint*)> deleteVertexArrays = nullptr;
	std::function<void (GLuint)> bindVertexArray = nullptr;

	const char* lastCallFile = "";
	int lastCallLine = -1;

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
	QOpenGLFunctions_2_0* gl(const char* file, int line)
	{
		checkGLErrors(file, line);
		assert(functions20);
		return functions20;
	}
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

	/**
	 * @brief Checks if there were any OpenGL errors.
	 *
	 * If yes, then print a message to log. Unless the only error detected is GL_OUT_OF_MEMORY,
	 * throw an exception.
	 *
	 * At most 10 messages are printed so that the output and log doesn't get flooded.
	 * This can happen on some platforms, where (for some reason) glGetError() never stops returning errors.
	 * In extreme cases this can lead to filling of the whole harddrive with the same error message.
	 *
	 * @return True if GL_OUT_OF_MEMORY was detected, false otherwise.
	 */
	bool checkGLErrors();

	/**
	 * @brief Call oveloaded checkGLErrors() and store last call position.
	 */
	bool checkGLErrors(const char* file, int line)
	{
		bool ret = checkGLErrors();
		lastCallFile = file;
		lastCallLine = line;
		return ret;
	}

	// Part of OpenGL 3.0.
	void glGenVertexArrays(GLsizei n, GLuint* arrays, const char* file, int line)
	{
		checkGLErrors(file, line);

		if (genVertexArrays)
		{
			genVertexArrays(n, arrays);
		}
		else
		{
			assert(functions30);
			functions30->glGenVertexArrays(n, arrays);
		}
	}
	#define glGenVertexArrays(a_, b_) glGenVertexArrays(a_, b_, __FILE__, __LINE__)

	void glDeleteVertexArrays(GLsizei n, const GLuint* arrays, const char* file, int line)
	{
		checkGLErrors(file, line);

		if (deleteVertexArrays)
		{
			deleteVertexArrays(n, arrays);
		}
		else
		{
			assert(functions30);
			functions30->glDeleteVertexArrays(n, arrays);
		}
	}
	#define glDeleteVertexArrays(a_, b_) glDeleteVertexArrays(a_, b_, __FILE__, __LINE__)

	void glBindVertexArray(GLuint array, const char* file, int line)
	{
		checkGLErrors(file, line);

		if (bindVertexArray)
		{
			bindVertexArray(array);
		}
		else
		{
			assert(functions30);
			functions30->glBindVertexArray(array);
		}
	}
	#define glBindVertexArray(a_) glBindVertexArray(a_, __FILE__, __LINE__)

	// Part of OpenGL 4.3.
	void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride, const char* file, int line)
	{
		checkGLErrors(file, line);
		assert(functions43);
		functions43->glBindVertexBuffer(bindingindex, buffer, offset, stride);
	}
	#define glBindVertexBuffer(a_, b_, c_, d_) glBindVertexBuffer(a_, b_, c_, d_, __FILE__, __LINE__)

	void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset, const char* file, int line)
	{
		checkGLErrors(file, line);
		assert(functions43);
		functions43->glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
	}
	#define glVertexAttribFormat(a_, b_, c_, d_, e_) glVertexAttribFormat(a_, b_, c_, d_, e_, __FILE__, __LINE__)

	void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex, const char* file, int line)
	{
		checkGLErrors(file, line);
		assert(functions43);
		functions43->glVertexAttribBinding(attribindex, bindingindex);
	}
	#define glVertexAttribBinding(a_, b_) glVertexAttribBinding(a_, b_, __FILE__, __LINE__)

	void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor, const char* file, int line)
	{
		checkGLErrors(file, line);
		assert(functions43);
		functions43->glVertexBindingDivisor(bindingindex, divisor);
	}
	#define glVertexBindingDivisor(a_, b_) glVertexBindingDivisor(a_, b_, __FILE__, __LINE__)

private:
	/**
	 * @brief Converts enum item to a string name.
	 */
	std::string glErrorCodeToString(GLenum code);
};

#endif // OPENGLINTERFACE_H
