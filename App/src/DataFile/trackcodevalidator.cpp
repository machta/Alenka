#include "trackcodevalidator.h"

#include "../openclcontext.h"
#include "../options.h"
#include "../SignalProcessor/montage.h"

#include <string>

using namespace std;

TrackCodeValidator::TrackCodeValidator()
{
	context = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS);
}

TrackCodeValidator::~TrackCodeValidator()
{
	delete context;
}

bool TrackCodeValidator::validate(const QString& input, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		return Montage::test(input.toStdString(), context, nullptr);
	}
	else
	{
		string message;

		bool result = Montage::test(input.toStdString(), context, &message);

		*errorMessage = QString::fromStdString(message);

		return result;
	}
}