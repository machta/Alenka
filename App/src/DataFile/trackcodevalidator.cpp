#include "trackcodevalidator.h"

#include "../myapplication.h"
#include "../options.h"

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>

#include <string>

using namespace std;

TrackCodeValidator::TrackCodeValidator()
{
	context = globalContext.get();
}

TrackCodeValidator::~TrackCodeValidator()
{
}

bool TrackCodeValidator::validate(const QString& input, QString* errorMessage)
{
	/*if (errorMessage == nullptr)
	{
		return Montage::test(input.toStdString(), context, nullptr);
	}
	else
	{
		string message;

		bool result = Montage::test(input.toStdString(), context, &message);

		*errorMessage = QString::fromStdString(message);

		return result;
	}*/
	return true; // TODO: fix this test
}
