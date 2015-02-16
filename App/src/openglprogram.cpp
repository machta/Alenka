#include "openglprogram.h"

#include "error.h"

#include <cstdio>

using namespace std;

#define fun() fun_shortcut()

OpenGLProgram::OpenGLProgram(const char* vertSource, const char* fragSource)
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
		GLchar* log = new GLchar[logLength];
		fun()->glGetProgramInfoLog(program, logLength, &logLength, log);

		cerr << "OpenGLProgram ('" << vertSource << "', '" << fragSource << "') link log:" << endl << log;

		delete log;
	}
#endif

	checkNotErrorCode(linkStatus, GL_FALSE, "OpenGLProgram ('" << vertSource << "', '" << fragSource << "') link failed.");
}

OpenGLProgram::~OpenGLProgram()
{
	fun()->glDeleteProgram(program);
}

GLchar* OpenGLProgram::readSource(const char* filePath)
{
	FILE* file = fopen(filePath, "r");
	checkNotErrorCode(file, nullptr, "File '" << filePath << "' could not be opened.");

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);

	GLchar* source = new GLchar[size + 1];
	source[size] = 0;

	rewind(file);
	freadChecked(source, sizeof(char), size, file);

	fclose(file);

	return source;
}

void OpenGLProgram::addShader(GLuint program, const char* filePath, GLenum type)
{
	GLuint shader = fun()->glCreateShader(type);

	GLchar* sourceText = readSource(filePath);
	fun()->glShaderSource(shader, 1, &sourceText, nullptr);
	delete[] sourceText;

	fun()->glCompileShader(shader);

	GLint compileStatus;
	fun()->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

#ifndef NDEBUG
	GLint logLength;
	fun()->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		GLchar* log = new GLchar[logLength];
		fun()->glGetShaderInfoLog(shader, logLength, &logLength, log);

		cerr << "Shader '" << filePath << "' compilation log:" << endl << log;

		delete log;
	}
#endif

	checkNotErrorCode(compileStatus, GL_FALSE, "Shader '" << filePath << "' compilation failed.");

	fun()->glAttachShader(program, shader);

	fun()->glDeleteShader(shader);
}

#undef fun
