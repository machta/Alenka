#ifndef TRACKCODEVALIDATOR_H
#define TRACKCODEVALIDATOR_H

#include <string>

class QString;

namespace AlenkaSignal
{
class OpenCLContext;
}

/**
 * @brief A convenience class for testing montage track code.
 */
class TrackCodeValidator
{
public:
	TrackCodeValidator();

	/**
	 * @brief Test the code in input.
	 * @param input Input code.
	 * @param errorMessage [out]
	 * @return True if the test succeeds.
	 */
	bool validate(const QString& input, QString* errorMessage = nullptr);

private:
	AlenkaSignal::OpenCLContext* context;
	std::string header;
};

#endif // TRACKCODEVALIDATOR_H
