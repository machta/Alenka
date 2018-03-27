#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include <QObject>

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "infotable.h"
#include "kernelcache.h"

#include <memory>
#include <vector>

class UndoCommandFactory;

class OpenDataFile {
public:
  AlenkaFile::DataFile *file = nullptr;
  const AlenkaFile::DataModel *dataModel = nullptr;
  UndoCommandFactory *undoFactory = nullptr;

  static InfoTable infoTable;
  static std::unique_ptr<KernelCache> kernelCache;
};

#endif // OPENDATAFILE_H
