#include "openglinterface.h"

#include "error.h"

#include <QOpenGLContext>
#include <QOpenGLDebugLogger>

#include <detailedexception.h>

using namespace std;

OpenGLInterface::~OpenGLInterface() {}

void OpenGLInterface::initializeOpenGLInterface() {
  logToFile("Initializing OpenGLInterface.");

  QOpenGLContext *c = QOpenGLContext::currentContext();
  checkErrorCode(c->isValid(), true, "is current context valid");

  functions20 = c->versionFunctions<QOpenGLFunctions_2_0>();
  checkNotErrorCode(functions20, nullptr,
                    "versionFunctions<QOpenGLFunctions_2_0>() failed.");
  checkNotErrorCode(functions20->initializeOpenGLFunctions(), false,
                    "initializeOpenGLFunctions() failed.");

  if (!programOption<bool>("gl20")) {
    functions30 = c->versionFunctions<QOpenGLFunctions_3_0>();
    checkNotErrorCode(functions30, nullptr,
                      "versionFunctions<QOpenGLFunctions_3_0>() failed.");
    checkNotErrorCode(functions30->initializeOpenGLFunctions(), false,
                      "initializeOpenGLFunctions() failed.");
  }

  if (programOption<bool>("gl43")) {
    functions43 = c->versionFunctions<QOpenGLFunctions_4_3_Core>();
    checkNotErrorCode(functions43, nullptr,
                      "versionFunctions<QOpenGLFunctions_4_3_Core>() failed.");
    checkNotErrorCode(functions43->initializeOpenGLFunctions(), false,
                      "initializeOpenGLFunctions() failed.");
  }

  // Initialize log.
  logger = make_unique<QOpenGLDebugLogger>();

  if (logger->initialize()) {
    logger->logMessage(QOpenGLDebugMessage::createApplicationMessage(
        "OpenGL debug log initialized."));
  } else {
    cerr << "Warning: initialization of QOpenGLDebugLogger failed" << endl;
  }
}

bool OpenGLInterface::checkGLErrors() {
  GLenum err;
  int errorsDetected = 0;
  int outOfMemoryErrorsDetected = 0;

  while (err = functions20->glGetError(),
         err != GL_NO_ERROR && errorsDetected <= 10) {
    if (err == GL_OUT_OF_MEMORY) {
      ++outOfMemoryErrorsDetected;
    } else {
      ++errorsDetected;

      logToFileAndConsole("OpenGL error: " << glErrorCodeToString(err)
                                           << " last call from " << lastCallFile
                                           << ":" << lastCallLine);
    }
  }

  if (errorsDetected)
    throwDetailed(
        runtime_error(to_string(errorsDetected) + " OpenGL errors detected."));

  return 0 < outOfMemoryErrorsDetected;
}

string OpenGLInterface::glErrorCodeToString(GLenum code) {
#define CASE(a_)                                                               \
  case a_:                                                                     \
    return #a_

  switch (code) {
    CASE(GL_INVALID_ENUM);
    CASE(GL_INVALID_VALUE);
    CASE(GL_INVALID_OPERATION);
    CASE(GL_STACK_OVERFLOW);
    CASE(GL_STACK_UNDERFLOW);
    CASE(GL_OUT_OF_MEMORY);
  }

#undef CASE

  return "unknown code " + errorCodeToString(code);
}

unique_ptr<OpenGLInterface> OPENGL_INTERFACE;
