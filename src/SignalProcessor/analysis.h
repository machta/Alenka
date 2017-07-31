#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QWidget>

#include <string>

class OpenDataFile;

class Analysis {
public:
  virtual void runAnalysis(OpenDataFile *file, QWidget *parent) = 0;
  virtual std::string name() = 0;
};

#endif // ANALYSIS_H
