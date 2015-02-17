#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

class OpenGLProgram : public OpenGLInterface
{
public:
	OpenGLProgram(const char* vertSource, const char* fragSource);
	~OpenGLProgram();

	GLuint getGLProgram() const
	{
		return program;
	}

private:
	GLuint program;

	GLchar* readSource(const char* filePath);
	void addShader(GLuint program, const char* filePath, GLenum type);
};

#endif // OPENGLPROGRAM_H
