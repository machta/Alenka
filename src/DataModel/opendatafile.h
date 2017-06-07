#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include <QObject>

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "infotable.h"

#include <vector>

class UndoCommandFactory;
class KernelCache;

class OpenDataFile {
public:
  AlenkaFile::DataFile *file = nullptr;
  const AlenkaFile::DataModel *dataModel = nullptr;
  UndoCommandFactory *undoFactory = nullptr;
  KernelCache *kernelCache = nullptr;

  static InfoTable infoTable;
};

#endif // OPENDATAFILE_H
