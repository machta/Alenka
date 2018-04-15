#include "filetype.h"

#include <QFileInfo>

#ifdef USE_BIOSIG
#include "../Alenka-File/include/AlenkaFile/biosigfile.h"
#endif // USE_BIOSIG

#include "../Alenka-File/include/AlenkaFile/edf.h"
#include "../Alenka-File/include/AlenkaFile/eep.h"
#include "../Alenka-File/include/AlenkaFile/gdf2.h"
#include "../Alenka-File/include/AlenkaFile/mat.h"

#include "options.h"

using namespace std;
using namespace AlenkaFile;

namespace {

class GdfFileType : public FileType {
  QString fileName;

public:
  GdfFileType(const QString &fileName) : fileName(fileName) {}

  unique_ptr<DataFile> makeInstance() override {
    return make_unique<GDF2>(fileName.toStdString(),
                             programOption<bool>("uncalibratedGDF"));
  }

  QString name() override {
    return "Alenka's internal implementation of GDFv2";
  }
};

class EdfFileType : public FileType {
  QString fileName;

public:
  EdfFileType(const QString &fileName) : fileName(fileName) {}

  unique_ptr<DataFile> makeInstance() override {
    return make_unique<EDF>(fileName.toStdString());
  }

  QString name() override { return "EDFLib"; }
};

class MatFileType : public FileType {
  QString fileName;
  vector<string> additionalFiles;

public:
  MatFileType(const QString &fileName, const vector<string> &additionalFiles)
      : fileName(fileName), additionalFiles(additionalFiles) {}

  unique_ptr<DataFile> makeInstance() override {
    MATvars vars;

    if (isProgramOptionSet("matData"))
      programOption("matData", vars.data);

    programOption("matFs", vars.frequency);
    programOption("matMults", vars.multipliers);
    programOption("matDate", vars.date);
    programOption("matLabel", vars.label);
    programOption("matEvtPos", vars.eventPosition);
    programOption("matEvtDur", vars.eventDuration);
    programOption("matEvtChan", vars.eventChannel);

    vector<string> files{fileName.toStdString()};
    files.insert(files.end(), additionalFiles.begin(), additionalFiles.end());

    return make_unique<MAT>(fileName.toStdString(), vars);
  }

  QString name() override { return "Alenka's internal implementation of MAT"; }
};

class EepFileType : public FileType {
  QString fileName;

public:
  EepFileType(const QString &fileName) : fileName(fileName) {}

  unique_ptr<DataFile> makeInstance() override {
    return make_unique<EEP>(fileName.toStdString());
  }

  QString name() override { return "LibEEP for CNT (ANT) and AVR"; }
};

#ifdef USE_BIOSIG

class BioSigFileType : public FileType {
  QString fileName;

public:
  BioSigFileType(const QString &fileName) : fileName(fileName) {}

  unique_ptr<DataFile> makeInstance() override {
    return make_unique<BioSigFile>(fileName.toStdString());
  }

  QString name() override { return "BioSig"; }
};

#endif // USE_BIOSIG

} // namespace

// TODO: Add BDF support through Edflib.

vector<unique_ptr<FileType>>
FileType::fromSuffix(const QString &fileName,
                     const vector<string> &additionalFiles) {
  QFileInfo fileInfo(fileName);
  QString suffix = fileInfo.suffix().toLower();

  vector<unique_ptr<FileType>> result;

  if (suffix == "gdf") {
    result.emplace_back(make_unique<GdfFileType>(fileName));
#ifdef USE_BIOSIG
    result.emplace_back(make_unique<BioSigFileType>(fileName));
#endif // USE_BIOSIG
  } else if (suffix == "edf") {
    result.emplace_back(make_unique<EdfFileType>(fileName));
  } else if (suffix == "mat") {
    result.emplace_back(make_unique<MatFileType>(fileName, additionalFiles));
  } else if (suffix == "cnt") {
    result.emplace_back(make_unique<EepFileType>(fileName));
  } else {
#ifdef USE_BIOSIG
    result.emplace_back(make_unique<BioSigFileType>(fileName));
#endif // USE_BIOSIG
  }

  return result;
}
