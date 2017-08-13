#ifndef TRACKCODEVALIDATOR_H
#define TRACKCODEVALIDATOR_H

#include <QString>

#include <string>

namespace AlenkaSignal {
class OpenCLContext;
}

/**
 * @brief A convenience class for testing montage track code.
 */
class TrackCodeValidator {
  AlenkaSignal::OpenCLContext *context;

public:
  TrackCodeValidator();

  /**
   * @brief Test the code in input.
   * @param input Input code.
   * @param errorMessage [out]
   * @param input Input OpenCL header.
   * @return True if the test succeeds.
   */
  bool validate(const QString &input, const QString &header,
                QString *errorMessage = nullptr);
};

#endif // TRACKCODEVALIDATOR_H
