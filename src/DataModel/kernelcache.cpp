#include "kernelcache.h"

#include "../error.h"
#include "../myapplication.h"
#include "../options.h"

#include <QFile>

#include <fstream>
#include <sstream>

using namespace std;

namespace {

string cacheFilePath() {
  int platform, device;
  programOption("clPlatform", platform);
  programOption("clDevice", device);

  const QString fileName = QString::fromStdString(
      "kernel-cache-" + to_string(platform) + '-' + to_string(device) + ".txt");

  QString path;
  if (isProgramOptionSet("kernelCacheDir")) {
    string tmp;
    programOption("kernelCacheDir", tmp);
    path = MyApplication::makeSubdir(QString::fromStdString(tmp), {fileName})
               .absolutePath();
  } else {
    path = MyApplication::makeAppSubdir({fileName}).absolutePath();
  }

  return path.toStdString();
}

} // namespace

// TODO: Perhaps use boost/compute/detail/lru_cache.hpp instead.
// TODO: Reject caches from old Alenka versions and from changed devices.
KernelCache::KernelCache() {
  cache.setMaxCost(programOption<int>("kernelCacheSize"));
}

void KernelCache::loadFromFile(AlenkaSignal::OpenCLContext *const context) {
  const string filePath = cacheFilePath();
  ifstream file(filePath, ios::binary);

  if (file.is_open()) {
    // This cannot be done earlier, because we don't want is_open() to throw.
    file.exceptions(ifstream::failbit | ifstream::badbit);

    int size;
    file >> size;
    logToFileAndConsole("Loading " + to_string(size) + " KernelCache entries");

    for (int i = 0; i < size; ++i) {
      size_t codeSize;
      file >> codeSize;
      file.ignore(2, '\n');

      auto code = make_unique<char[]>(codeSize + 1);
      file.read(code.get(), codeSize);

      size_t binarySize;
      file >> binarySize;
      file.ignore(2, '\n');

      auto binary = make_unique<vector<unsigned char>>(binarySize);
      file.read(reinterpret_cast<char *>(binary->data()), binarySize);

      auto program =
          AlenkaSignal::OpenCLProgram::fromBinary(binary.get(), context);
      cache.insert(QString(code.get()), program);
    }
  } else {
    logToFileAndConsole("Failed to open kernel cache at " << filePath << ".");
  }
}

void KernelCache::saveToFile() {
  const int size = cache.size();
  if (size <= 0)
    return;

  const string filePath = cacheFilePath();
  ofstream file(filePath, ios::binary);

  if (file.is_open()) {
    // This cannot be done earlier, because we don't want is_open() to throw.
    file.exceptions(ifstream::failbit | ifstream::badbit);

    logToFileAndConsole("Saving " + to_string(size) + " KernelCache entries");
    file << size << endl;
    const auto &keys = cache.keys();

    for (auto qCode : keys) {
      size_t codeSize = qCode.size();
      string code = qCode.toStdString();

      file << codeSize << endl;
      file.write(code.data(), codeSize) << endl;

      auto program = find(qCode);
      auto binary = program->getBinary();
      size_t binarySize = binary->size();

      file << binarySize << endl;
      file.write(reinterpret_cast<const char *>(binary->data()), binarySize)
          << endl;
    }
  } else {
    logToFileAndConsole("Failed to save kernel cache to " << filePath << ".");
  }
}

void KernelCache::deleteCacheFile() {
  // Right now the file is not deleted when persistent storage is disabled. So
  // this function is never used.
  assert(false);
  QFile::remove(QString::fromStdString(cacheFilePath()));
}
