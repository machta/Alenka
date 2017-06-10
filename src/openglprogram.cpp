#include "openglprogram.h"

#include "error.h"

#include <cstdio>
#include <sstream>

using namespace std;

OpenGLProgram::OpenGLProgram(const string &vertSource,
                             const string &fragSource) {
  program = gl()->glCreateProgram();

  string shaderHeader;
  if (programOption<bool>("gl20"))
    shaderHeader = "#version 110\n#define GLSL_110\n";
  else
    shaderHeader = "#version 130\n";

  addShader(shaderHeader + vertSource, GL_VERTEX_SHADER);
  addShader(shaderHeader + fragSource, GL_FRAGMENT_SHADER);

  gl()->glLinkProgram(program);

  GLint linkStatus;
  gl()->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

#ifndef NDEBUG
  GLint logLength;
  gl()->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

  if (logLength > 1) {
    auto log = make_unique<char[]>(logLength);
    gl()->glGetProgramInfoLog(program, logLength, &logLength, log.get());

    logToFileAndConsole("OpenGLProgram link log:" << endl << log.get());
  }
#endif

  checkNotErrorCode(linkStatus, GL_FALSE, "OpenGLProgram link failed.");
}

OpenGLProgram::~OpenGLProgram() {
  gl()->glDeleteProgram(program);

  OPENGL_INTERFACE->checkGLErrors();
}

void OpenGLProgram::addShader(const string &sourceText, GLenum type) {
  GLuint shader = gl()->glCreateShader(type);

  const char *source = sourceText.c_str();
  gl()->glShaderSource(shader, 1, &source, nullptr);

  gl()->glCompileShader(shader);

  GLint compileStatus;
  gl()->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

#ifndef NDEBUG
  logCompilationInfo(shader, type, sourceText);
#endif

  checkNotErrorCode(compileStatus, GL_FALSE,
                    "Shader " << type << " compilation failed.");

  gl()->glAttachShader(program, shader);

  gl()->glDeleteShader(shader);
}

void OpenGLProgram::logCompilationInfo(GLuint shader, GLenum type,
                                       const string &sourceText) {
  GLint logLength;
  gl()->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

  if (logLength > 1) {
    auto log = make_unique<char[]>(logLength);
    gl()->glGetShaderInfoLog(shader, logLength, &logLength, log.get());

    stringstream ss;
    switch (type) {
    case GL_VERTEX_SHADER:
      ss << "GL_VERTEX_SHADER"
         << "(" << type << ")";
      break;
    case GL_FRAGMENT_SHADER:
      ss << "GL_FRAGMENT_SHADER"
         << "(" << type << ")";
      break;
    default:
      ss << type;
      break;
    }

    logToFileAndConsole("Shader " << ss.str() << ":" << endl
                                  << sourceText << endl
                                  << "Compilation log:" << endl
                                  << log.get());
  }
}
