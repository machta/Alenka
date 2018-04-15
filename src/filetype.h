#ifndef FILETYPE_H
#define FILETYPE_H

#include <memory>
#include <string>
#include <vector>

#include <QString>

#include "../Alenka-File/include/AlenkaFile/datafile.h"

/**
 * @brief This factory class is used produce instances of DataFiles.
 */
class FileType {
public:
  virtual std::unique_ptr<AlenkaFile::DataFile> makeInstance() = 0;
  virtual QString name() = 0;

  static std::vector<std::unique_ptr<FileType>>
  fromSuffix(const QString &fileName,
             const std::vector<std::string> &additionalFiles);
};

#endif // FILETYPE_H
