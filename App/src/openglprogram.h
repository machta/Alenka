#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

#include "error.h"

#include <string>

/**
 * @brief OpenGL program object wrapper.
 */
class OpenGLProgram : public OpenGLInterface
{
public:
	/**
	 * @brief Constructor taking file streams as shader sources.
	 */
	OpenGLProgram(FILE* vertSource, FILE* fragSource)
	{
		construct(readWholeTextFile(vertSource), readWholeTextFile(fragSource));
	}

	/**
	 * @brief Constructor taking strings as shader sources.
	 */
	OpenGLProgram(const std::string& vertSource, const std::string& fragSource)
	{
		construct(vertSource, fragSource);
	}

	~OpenGLProgram();

	/**
	 * @brief Used to retrieve the underlying program object.
	 */
	GLuint getGLProgram() const
	{
		return program;
	}

private:
	GLuint program;

	/**
	 * @brief Common constructor functionality.
	 */
	void construct(const std::string& vertSource, const std::string& fragSource);

	/**
	 * @brief Compile a shader and attach it to the program.
	 */
	void addShader(const std::string& sourceText, GLenum type);
};

#endif // OPENGLPROGRAM_H
