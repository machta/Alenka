#include "trackcodevalidator.h"

#include "../SignalProcessor/signalprocessor.h"
#include "../myapplication.h"

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"

#include <QFile>

using namespace std;
using namespace AlenkaSignal;

TrackCodeValidator::TrackCodeValidator() {
  context = globalContext.get();

  QFile headerFile(":/montageHeader.cl");
  headerFile.open(QIODevice::ReadOnly);
  header = headerFile.readAll().toStdString();
}

bool TrackCodeValidator::validate(const QString &input, QString *errorMessage) {
  string code = SignalProcessor::simplifyMontage<float>(input.toStdString());

  if (errorMessage) {
    string message;

    // I don't pass the labels here, because a montage formula with a bad label
    // doesn't violate the syntax. Instead a fall back to zero is used. This is
    // consistent with how an invalid index is handled.
    bool result = Montage<float>::test(code, context, &message, header);

    *errorMessage = QString::fromStdString(message);

    return result;
  } else {
    return Montage<float>::test(code, context, nullptr, header);
  }

  return true;
}
