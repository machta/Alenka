#include "openglprogram.h"

#include <cstdio>
#include <sstream>

using namespace std;

OpenGLProgram::~OpenGLProgram()
{
	gl()->glDeleteProgram(program);

	gl();
}

void OpenGLProgram::construct(const string& vertSource, const string& fragSource)
{
	initializeOpenGLInterface();

	program = gl()->glCreateProgram();

#if defined GL_2_0
	const char* shaderHeader =
		"#version 110\n"
		"#extension GL_EXT_gpu_shader4 : enable\n\n"
		"#define GL_2_0\n";
#elif defined GL_3_0
	const char* shaderHeader =
		"#version 130\n\n"
		"#define GL_3_0\n";
#elif defined GL_3_2
	const char* shaderHeader =
		"#version 150 core\n\n"
		"#define GL_3_2\n";
#endif

	addShader(shaderHeader + vertSource, GL_VERTEX_SHADER);
	addShader(shaderHeader + fragSource, GL_FRAGMENT_SHADER);

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

		logToFileAndConsole("OpenGLProgram link log:" << endl << log);

		delete log;
	}
#endif

	checkNotErrorCode(linkStatus, GL_FALSE, "OpenGLProgram link failed.");
}

void OpenGLProgram::addShader(const string& sourceText, GLenum type)
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

		stringstream ss;
		switch (type)
		{
		case GL_VERTEX_SHADER:
			ss << "GL_VERTEX_SHADER" << "(" << type << ")";
			break;
		case GL_FRAGMENT_SHADER:
			ss << "GL_FRAGMENT_SHADER" << "(" << type << ")";
			break;
		default:
			ss << type;
			break;
		}

		logToFileAndConsole("Shader " << ss.str() << ":" << endl << sourceText << endl << "Compilation log:" << endl << log);

		delete log;
	}
#endif

	checkNotErrorCode(compileStatus, GL_FALSE, "Shader " << type << " compilation failed.");

	gl()->glAttachShader(program, shader);

	gl()->glDeleteShader(shader);
}
