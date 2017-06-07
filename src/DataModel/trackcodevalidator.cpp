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

    bool result = Montage<float>::test(code, context, &message, header);

    *errorMessage = QString::fromStdString(message);

    return result;
  } else {
    return Montage<float>::test(code, context, nullptr, header);
  }

  return true;
}
