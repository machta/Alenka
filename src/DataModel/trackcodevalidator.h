#ifndef TRACKCODEVALIDATOR_H
#define TRACKCODEVALIDATOR_H

#include <string>

class QString;

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
   * @return True if the test succeeds.
   */
  bool validate(const QString &input, QString *errorMessage = nullptr);
};

#endif // TRACKCODEVALIDATOR_H
