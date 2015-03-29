#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

#include "error.h"

#include <string>

class OpenGLProgram : public OpenGLInterface
{
public:
	OpenGLProgram(FILE* vertSource, FILE* fragSource)
	{
		construct(readWholeTextFile(vertSource), readWholeTextFile(fragSource));
	}
	OpenGLProgram(const std::string& vertSource, const std::string& fragSource)
	{
		construct(vertSource, fragSource);
	}
	~OpenGLProgram();

	GLuint getGLProgram() const
	{
		return program;
	}

private:
	GLuint program;

	void construct(const std::string& vertSource, const std::string& fragSource);
	void addShader(GLuint program, const std::string& sourceText, GLenum type);
};

#endif // OPENGLPROGRAM_H
