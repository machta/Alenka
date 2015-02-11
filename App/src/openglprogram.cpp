#include "openglprogram.h"

#include <cstdio>
#include <stdexcept>
#include <sstream>

using namespace std;

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

	if (linkStatus == GL_FALSE)
	{
		stringstream ss;
		ss << "OpenGLProgram ('" << vertSource << "', '" << fragSource << "') link failed.";
		throw runtime_error(ss.str());
	}
}

OpenGLProgram::~OpenGLProgram()
{
	fun()->glDeleteProgram(program);
}

GLchar* OpenGLProgram::readSource(const char* filePath)
{
	FILE* file = fopen(filePath, "r");
	if (file == nullptr)
	{
		stringstream ss;
		ss << "File '" << filePath << "' could not be opened.";
		throw runtime_error(ss.str());
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);

	GLchar* source = new GLchar[size + 1];
	source[size] = 0;

	rewind(file);
	fread(source, sizeof(char), size, file);

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

	if (compileStatus == GL_FALSE)
	{
		stringstream ss;
		ss << "Shader '" << filePath << "' compilation failed.";
		throw runtime_error(ss.str());
	}

	fun()->glAttachShader(program, shader);

	fun()->glDeleteShader(shader);
}

