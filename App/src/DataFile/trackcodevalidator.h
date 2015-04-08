#ifndef TRACKCODEVALIDATOR_H
#define TRACKCODEVALIDATOR_H

class OpenCLContext;
class QString;

class TrackCodeValidator
{
public:
	TrackCodeValidator();
	~TrackCodeValidator();

	bool validate(const QString& input, QString* errorMessage = nullptr);

private:
	OpenCLContext* context;
};

#endif // TRACKCODEVALIDATOR_H
