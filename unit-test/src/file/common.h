#include "../../Alenka-File/include/AlenkaFile/biosigfile.h"
#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../../Alenka-File/include/AlenkaFile/datamodel.h"
#include "../../Alenka-File/include/AlenkaFile/edf.h"
#include "../../Alenka-File/include/AlenkaFile/gdf2.h"
#include "../../Alenka-File/include/AlenkaFile/mat.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace AlenkaFile;

const string TEST_DATA_PATH = TEST_DATA + string("/");

const double MAX_REL_ERR_DOUBLE = 0.0001;
const double MAX_REL_ERR_FLOAT = 0.0001;
const double MAX_ABS_ERR_DOUBLE = 0.000001;
const double MAX_ABS_ERR_FLOAT = 0.01;

class TestFile {
  bool hasValues = false;
  vector<double> values;

public:
  string path;
  double sampleRate;
  unsigned int channelCount, samplesRecorded;

  TestFile(string path, double sampleRate = 0, int channelCount = 0,
           int samplesRecorded = 0)
      : path(std::move(path)), sampleRate(sampleRate),
        channelCount(channelCount), samplesRecorded(samplesRecorded) {}
  ~TestFile() = default;

  DataFile *makeGDF2() { return new GDF2(path); }
  DataFile *makeEDF() { return new EDF(path); }
  template <class... T> DataFile *makeMAT(T... p) {
    return new MAT(path, p...);
  }
  DataFile *makeBioSigFile() { return new BioSigFile(path); }

  const vector<double> &getValues() {
    if (hasValues == false) {
      hasValues = true;

      fstream valuesFile;
      valuesFile.open(stripExtension(path) + "_values.dat");

      while (valuesFile) {
        double sample;
        if (valuesFile >> sample)
          values.push_back(sample);
      }
    }

    return values;
  }

private:
  string stripExtension(const string &path) {
    return path.substr(0, path.find_last_of('.'));
  }
};

template <class T> void fillVector(vector<T> &v, T val) {
  for (auto &e : v)
    e = val;
}

template <class T, class U>
void compareMatrix(T *arr, U *sol, int rows, int cols, int arrRowLen,
                   int solRowLen, double *relErr, double *absErr) {
  double rel = 0;
  double abs = 0;

  for (int j = 0; j < rows; ++j) {
    for (int i = 0; i < cols; ++i) {
      double diff = fabs(arr[i] - sol[i]);
      abs = max<double>(abs, diff);
      if (sol[i] != 0)
        rel = max<double>(rel, fabs(diff / sol[i]));
    }

    arr += arrRowLen;
    sol += solRowLen;
  }

  *relErr = rel;
  *absErr = abs;
}

template <class T, class U>
void compareMatrix(T *arr, U *sol, int rows, int cols, double *relErr,
                   double *absErr) {
  compareMatrix(arr, sol, rows, cols, cols, cols, relErr, absErr);
}

template <class T, class U>
void compareMatrixAverage(T *arr, U *sol, int rows, int cols, int arrRowLen,
                          int solRowLen, double *relErr, double *absErr) {
  double rel = 0;
  double abs = 0;

  for (int j = 0; j < rows; ++j) {
    for (int i = 0; i < cols; ++i) {
      double diff = fabs(arr[i] - sol[i]);
      abs += diff;
      if (sol[i] != 0)
        rel += fabs(diff / sol[i]);
    }

    arr += arrRowLen;
    sol += solRowLen;
  }

  int count = rows * cols;
  *relErr = rel / count;
  *absErr = abs / count;
}

template <class T, class U>
void compareMatrixAverage(T *arr, U *sol, int rows, int cols, double *relErr,
                          double *absErr) {
  compareMatrixAverage(arr, sol, rows, cols, cols, cols, relErr, absErr);
}

inline void printException(function<void(void)> fun) {
  try {
    fun();
  } catch (const exception &e) {
    cerr << "Caught an std exception: " << e.what() << endl;
    throw;
  } catch (...) {
    cerr << "Caught an exception." << endl;
    throw;
  }
}
