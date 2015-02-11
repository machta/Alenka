#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

class OpenGLProgram : public OpenGLInterface
{
public:
	OpenGLProgram(const char* vertSource, const char* fragSource);
	~OpenGLProgram();

	GLuint getGLProgram()
	{
		return program;
	}

private:
	GLuint program;

	void addShader(GLuint program, const char* filePath, GLenum type);
	GLchar* readSource(const char* filePath);
};

#endif // OPENGLPROGRAM_H
