#ifndef KERNELCACHE_H
#define KERNELCACHE_H

#include <QCache>

#include <vector>

#include "../Alenka-Signal/include/AlenkaSignal/openclprogram.h"

class KernelCache {
  QCache<QString, AlenkaSignal::OpenCLProgram> cache;

public:
  KernelCache();

  void insert(const QString &code, AlenkaSignal::OpenCLProgram *program) {
    cache.insert(code, program);
  }
  AlenkaSignal::OpenCLProgram *find(const QString &code) const {
    return cache[code];
  }

  void loadFromFile(AlenkaSignal::OpenCLContext *context);
  void saveToFile();

  static void deleteCacheFile();
};

#endif // KERNELCACHE_H
