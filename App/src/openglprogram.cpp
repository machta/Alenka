#include "openglprogram.h"

#include <cstdio>

using namespace std;

OpenGLProgram::~OpenGLProgram()
{
	gl()->glDeleteProgram(program);

	gl();
}

void OpenGLProgram::construct(const string& vertSource, const string& fragSource)
{
	program = gl()->glCreateProgram();

	addShader(program, vertSource, GL_VERTEX_SHADER);
	addShader(program, fragSource, GL_FRAGMENT_SHADER);

	gl()->glLinkProgram(program);

	GLint linkStatus;
	gl()->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

#ifndef NDEBUG
	GLint logLength;
	gl()->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		char* log = new char[logLength];
		gl()->glGetProgramInfoLog(program, logLength, &logLength, log);

		logToBoth("OpenGLProgram link log:" << endl << log);

		delete log;
	}
#endif

	checkNotErrorCode(linkStatus, GL_FALSE, "OpenGLProgram link failed.");
}

void OpenGLProgram::addShader(GLuint program, const string& sourceText, GLenum type)
{
	GLuint shader = gl()->glCreateShader(type);

	const char* source = sourceText.c_str();
	gl()->glShaderSource(shader, 1, &source, nullptr);

	gl()->glCompileShader(shader);

	GLint compileStatus;
	gl()->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

#ifndef NDEBUG
	GLint logLength;
	gl()->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		char* log = new char[logLength];
		gl()->glGetShaderInfoLog(shader, logLength, &logLength, log);

		logToBoth("Shader " << type << " compilation log:" << endl << log);

		delete log;
	}
#endif

	checkNotErrorCode(compileStatus, GL_FALSE, "Shader " << type << " compilation failed.");

	gl()->glAttachShader(program, shader);

	gl()->glDeleteShader(shader);
}
