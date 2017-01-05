#include "trackcodevalidator.h"

#include "../myapplication.h"

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>

#include <QFile>

using namespace std;
using namespace AlenkaSignal;

TrackCodeValidator::TrackCodeValidator()
{
	context = globalContext.get();

	QFile headerFile(":/montageHeader.cl");
	headerFile.open(QIODevice::ReadOnly);
	header = headerFile.readAll().toStdString();
}

TrackCodeValidator::~TrackCodeValidator()
{}

bool TrackCodeValidator::validate(const QString& input, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		return Montage<float>::test(input.toStdString(), context, nullptr, header);
	}
	else
	{
		string message;

		bool result = Montage<float>::test(input.toStdString(), context, &message, header);

		*errorMessage = QString::fromStdString(message);

		return result;
	}
	return true;
}
