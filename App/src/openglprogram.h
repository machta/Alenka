#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

#include <string>

class OpenGLProgram : public OpenGLInterface
{
public:
	OpenGLProgram(FILE* vertSource, FILE* fragSource);
	OpenGLProgram(const std::string& vertSource, const std::string& fragSource)
	{
        construct(vertSource.c_str(), fragSource.c_str());
	}
	~OpenGLProgram();

	GLuint getGLProgram() const
	{
		return program;
	}

private:
	GLuint program;

	void construct(const char *vertSource, const char *fragSource);
	void addShader(GLuint program, const char* sourceText, GLenum type);
};

#endif // OPENGLPROGRAM_H
