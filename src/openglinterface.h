/**
 * @brief The header defining the OpenGLInterface class.
 *
 * A pointer called OPENGL_INTERFACE holds a global instance of this class
 * that can be used across the whole program. This is mainly to facilitate
 * tracking of call positions that are needed to accurately report errors.
 * This shouldn't cause any problems as OpenGL can only be used from a single
 * thread anyway.
 *
 * @file
 */

#ifndef OPENGLINTERFACE_H
#define OPENGLINTERFACE_H

#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_4_3_Core>

#include <cassert>
#include <memory>

class QOpenGLDebugLogger;

/**
 * @brief This is a convenience class for accessing OpenGL.
 *
 * This object must be initialized using initializeOpenGLInterface() after an
 * OpenGL context is created and made current.
 *
 * All the public functions must be called only from within an active OpenGL
 * context section (i.e. between makeCurrent() and doneCurrent()).
 *
 * @todo Force only one global instance via the singleton pattern.
 */
class OpenGLInterface {
  QOpenGLFunctions_2_0 *functions20 = nullptr;
  QOpenGLFunctions_3_0 *functions30 = nullptr;
  QOpenGLFunctions_4_3_Core *functions43 = nullptr;
  std::unique_ptr<QOpenGLDebugLogger> logger;
  const char *lastCallFile = "";
  int lastCallLine = -1;

public:
  ~OpenGLInterface();

  /**
   * @brief Delayed initialization function.
   */
  void initializeOpenGLInterface();

  /**
   * @brief Returns a pointer needed for accessing the OpenGL 2.0 API.
   *
   * You can use the parameters to track the call position in the code.
   * If an error occurs it will be reported by the next gl*() call.
   *
   * You can use gl() macro to fill the parameters automatically.
   */
  QOpenGLFunctions_2_0 *gl20(const char *file, int line) {
    checkGLErrors(file, line);
    assert(functions20);
    return functions20;
  }

  /**
   * @brief Returns a pointer needed for accessing the OpenGL 3.0 API.
   *
   * The macro alias is gl3().
   */
  QOpenGLFunctions_3_0 *gl30(const char *file, int line) {
    checkGLErrors(file, line);
    assert(functions30);
    return functions30;
  }

  /**
   * @brief Returns a pointer needed for accessing the OpenGL 4.3 API.
   *
   * The macro alias is gl4().
   */
  QOpenGLFunctions_4_3_Core *gl43(const char *file, int line) {
    checkGLErrors(file, line);
    assert(functions43);
    return functions43;
  }

  /**
   * @brief Returns a pointer needed to access the OpenGL debug log.
   *
   * This also checks OpenGL errors.
   *
   * @return Returns a nullptr if initialization of the log failed.
   *
   * @note For some reason the log is always empty.(I guess the OpenGL
   * implementation doesn't use it or perhaps I didn't initialize it properly.
   */
  QOpenGLDebugLogger *log() {
    checkGLErrors();
    return logger.get();
  }

  /**
   * @brief Checks whether glGetError() returned any errors.
   *
   * The error messages are written to log, then, unless the only error detected
   * is GL_OUT_OF_MEMORY, throw an exception.
   *
   * At most 10 messages at a time are printed so that the log doesn't get
   * flooded. This can happen on some platforms, where (for some reason)
   * glGetError() never stops returning the same error over and over. In extreme
   * cases this can lead to filling the hard drive with a huge log file.
   *
   * @return Whether GL_OUT_OF_MEMORY was detected.
   */
  bool checkGLErrors();

private:
  /**
   * @brief Converts OpenGL error enum items to strings.
   */
  std::string glErrorCodeToString(GLenum code);

  /**
   * @brief Check errors and store the call-position information.
   */
  bool checkGLErrors(const char *file, int line) {
    bool ret = checkGLErrors();
    lastCallFile = file;
    lastCallLine = line;
    return ret;
  }
};

extern std::unique_ptr<OpenGLInterface> OPENGL_INTERFACE;

#define gl() OPENGL_INTERFACE->gl20(__FILE__, __LINE__)
#define gl3() OPENGL_INTERFACE->gl30(__FILE__, __LINE__)
#define gl4() OPENGL_INTERFACE->gl43(__FILE__, __LINE__)

#endif // OPENGLINTERFACE_H
