#ifndef TRACKCODEVALIDATOR_H
#define TRACKCODEVALIDATOR_H

class OpenCLContext;
class QString;

/**
 * @brief A convenience class for testing montage track code.
 */
class TrackCodeValidator
{
public:
	TrackCodeValidator();
	~TrackCodeValidator();

	/**
	 * @brief Test the code in input.
	 * @param input Input code.
	 * @param errorMessage [out]
	 * @return True, if the test succeeds.
	 */
	bool validate(const QString& input, QString* errorMessage = nullptr);

private:
	OpenCLContext* context;
};

#endif // TRACKCODEVALIDATOR_H
