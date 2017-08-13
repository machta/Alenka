#include "trackcodevalidator.h"

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"
#include "../SignalProcessor/signalprocessor.h"
#include "../myapplication.h"
#include "opendatafile.h"

#include <QFile>

using namespace std;
using namespace AlenkaSignal;

TrackCodeValidator::TrackCodeValidator() { context = globalContext.get(); }

bool TrackCodeValidator::validate(const QString &input, const QString &header,
                                  QString *errorMessage) {
  string code = SignalProcessor::simplifyMontage<float>(input.toStdString());
  string stdHeader = header.toStdString();

  if (errorMessage) {
    string message;

    // I don't pass the labels here, because a montage formula with a bad label
    // doesn't violate the syntax. Instead a fall back to zero is used. This is
    // consistent with how an invalid index is handled.
    bool result =
        Montage<float>::testHeader(code, context, stdHeader, &message);

    *errorMessage = QString::fromStdString(message);

    return result;
  } else {
    return Montage<float>::testHeader(code, context, stdHeader, nullptr);
  }

  return true;
}
