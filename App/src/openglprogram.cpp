#include "openglprogram.h"

#include "error.h"

#include <cstdio>

using namespace std;

#define fun() fun_shortcut()

OpenGLProgram::OpenGLProgram(FILE* vertSource, FILE* fragSource)
{
	char* tmp[2];
	FILE* file[] = {vertSource, fragSource};

	for (int i = 0; i < 2; ++i)
	{
		fseek(file[i], 0, SEEK_END);
		size_t size = ftell(file[i]);

		tmp[i] = new char[size + 1];
		tmp[i][size] = 0;

		rewind(file[i]);
		freadChecked(tmp[i], sizeof(char), size, file[i]);
	}

	construct(tmp[0], tmp[1]);

	delete[] tmp[0];
	delete[] tmp[1];
}

OpenGLProgram::~OpenGLProgram()
{
	fun()->glDeleteProgram(program);

	fun();
}

void OpenGLProgram::construct(const char* vertSource, const char* fragSource)
{
	program = fun()->glCreateProgram();

	addShader(program, vertSource, GL_VERTEX_SHADER);
	addShader(program, fragSource, GL_FRAGMENT_SHADER);

	fun()->glLinkProgram(program);

	GLint linkStatus;
	fun()->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

#ifndef NDEBUG
	GLint logLength;
	fun()->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		char* log = new char[logLength];
		fun()->glGetProgramInfoLog(program, logLength, &logLength, log);

		cerr << "OpenGLProgram ('" << vertSource << "', '" << fragSource << "') link log:" << endl << log;

		delete log;
	}
#endif

	checkNotErrorCode(linkStatus, GL_FALSE, "OpenGLProgram ('" << vertSource << "', '" << fragSource << "') link failed.");
}

void OpenGLProgram::addShader(GLuint program, const char* sourceText, GLenum type)
{
	GLuint shader = fun()->glCreateShader(type);

	fun()->glShaderSource(shader, 1, &sourceText, nullptr);

	fun()->glCompileShader(shader);

	GLint compileStatus;
	fun()->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

#ifndef NDEBUG
	GLint logLength;
	fun()->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		char* log = new char[logLength];
		fun()->glGetShaderInfoLog(shader, logLength, &logLength, log);

		cerr << "Shader " << type << " compilation log:" << endl << log;

		delete log;
	}
#endif

	checkNotErrorCode(compileStatus, GL_FALSE, "Shader " << type << " compilation failed.");

	fun()->glAttachShader(program, shader);

	fun()->glDeleteShader(shader);
}

#undef fun
