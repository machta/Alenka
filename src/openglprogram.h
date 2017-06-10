#ifndef OPENGLPROGRAM_H
#define OPENGLPROGRAM_H

#include "openglinterface.h"

#include <string>

/**
 * @brief OpenGL program object wrapper.
 */
class OpenGLProgram {
public:
  /**
   * @brief Constructor taking strings as shader sources.
   */
  OpenGLProgram(const std::string &vertSource, const std::string &fragSource);
  ~OpenGLProgram();

  /**
   * @brief Used to retrieve the underlying program object.
   */
  GLuint getGLProgram() const { return program; }

private:
  GLuint program;

  /**
   * @brief Compile a shader and attach it to the program.
   */
  void addShader(const std::string &sourceText, GLenum type);
  void logCompilationInfo(GLuint shader, GLenum type,
                          const std::string &sourceText);
};

#endif // OPENGLPROGRAM_H
