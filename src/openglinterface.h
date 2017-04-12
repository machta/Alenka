/**
 * @brief The header defining the OpenGLInterface class.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

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
	QOpenGLFunctions_2_0* functions = nullptr;
	QOpenGLDebugLogger* logger = nullptr;

	std::function<void (GLsizei, GLuint*)> genVertexArrays = nullptr;
	std::function<void (GLsizei, const GLuint*)> deleteVertexArrays = nullptr;
	std::function<void (GLuint)> bindVertexArray = nullptr;

	std::function<void (GLuint, GLuint, GLintptr, GLintptr)> bindVertexBuffer = nullptr;
	std::function<void (GLuint, GLint, GLenum, GLboolean, GLuint)> vertexAttribFormat = nullptr;
	std::function<void (GLuint, GLuint)> vertexAttribBinding = nullptr;
	std::function<void (GLuint, GLuint)> vertexBindingDivisor = nullptr;

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
		assert(functions);
		checkGLErrors(file, line);		
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

	// Part of OpenGL 3.0.
	void glGenVertexArrays(GLsizei n, GLuint* arrays, const char* file, int line)
	{
		assert(genVertexArrays);
		checkGLErrors(file, line);
		genVertexArrays(n, arrays);
	}
	#define glGenVertexArrays(a_, b_) glGenVertexArrays(a_, b_, __FILE__, __LINE__)

	void glDeleteVertexArrays(GLsizei n, const GLuint* arrays, const char* file, int line)
	{
		assert(deleteVertexArrays);
		checkGLErrors(file, line);
		deleteVertexArrays(n, arrays);
	}
	#define glDeleteVertexArrays(a_, b_) glDeleteVertexArrays(a_, b_, __FILE__, __LINE__)

	void glBindVertexArray(GLuint array, const char* file, int line)
	{
		assert(bindVertexArray);
		checkGLErrors(file, line);
		bindVertexArray(array);
	}
	#define glBindVertexArray(a_) glBindVertexArray(a_, __FILE__, __LINE__)

	// Part of OpenGL 4.3.
	void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride, const char* file, int line)
	{
		assert(bindVertexBuffer);
		checkGLErrors(file, line);
		bindVertexBuffer(bindingindex, buffer, offset, stride);
	}
	#define glBindVertexBuffer(a_, b_, c_, d_) glBindVertexBuffer(a_, b_, c_, d_, __FILE__, __LINE__)

	void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset, const char* file, int line)
	{
		assert(vertexAttribFormat);
		checkGLErrors(file, line);
		vertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
	}
	#define glVertexAttribFormat(a_, b_, c_, d_, e_) glVertexAttribFormat(a_, b_, c_, d_, e_, __FILE__, __LINE__)

	void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex, const char* file, int line)
	{
		assert(vertexAttribBinding);
		checkGLErrors(file, line);
		vertexAttribBinding(attribindex, bindingindex);
	}
	#define glVertexAttribBinding(a_, b_) glVertexAttribBinding(a_, b_, __FILE__, __LINE__)

	void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor, const char* file, int line)
	{
		assert(vertexBindingDivisor);
		checkGLErrors(file, line);
		vertexBindingDivisor(bindingindex, divisor);
	}
	#define glVertexBindingDivisor(a_, b_) glVertexBindingDivisor(a_, b_, __FILE__, __LINE__)

private:
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
	void checkGLErrors(const char* file, int line)
	{
		checkGLErrors();
		lastCallFile = file;
		lastCallLine = line;
	}

	/**
	 * @brief Converts enum item to a string name.
	 */
	std::string getErrorCode(GLenum code);
};

#endif // OPENGLINTERFACE_H
