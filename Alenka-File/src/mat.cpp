#include "../include/AlenkaFile/mat.h"

#include <matio.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include <detailedexception.h>

using namespace std;
using namespace AlenkaFile;

namespace {

template <class A, class B> void convertArray(A *a, B *b, int n) {
  for (int i = 0; i < n; ++i)
    b[i] = static_cast<B>(a[i]);
}

template <class B>
void decodeArray(void *a, B *b, matio_types type, int n = 1, int offset = 0) {
#define CASE(a_, b_)                                                           \
  case a_:                                                                     \
    convertArray(reinterpret_cast<b_ *>(a) + offset, b, n);                    \
    break;

  switch (type) {
    CASE(MAT_T_INT8, int8_t);
    CASE(MAT_T_UINT8, uint8_t);
    CASE(MAT_T_INT16, int16_t);
    CASE(MAT_T_UINT16, uint16_t);
    CASE(MAT_T_INT32, int32_t);
    CASE(MAT_T_UINT32, uint32_t);
    CASE(MAT_T_SINGLE, float);
    CASE(MAT_T_DOUBLE, double);
    CASE(MAT_T_INT64, int64_t);
    CASE(MAT_T_UINT64, uint64_t);
  default:
    runtime_error("Unsupported data type in MAT-file");
    break;
  }

#undef CASE
}

std::pair<string, string> splitVarName(const string &varName) {
  string firstPart;
  auto it = varName.begin();

  while (it != varName.end() && *it != '.')
    firstPart.push_back(*it++);

  string secondPart;
  if (firstPart.size() < varName.size())
    secondPart.assign(++it, varName.end());

  return make_pair(firstPart, secondPart);
}

matvar_t *readStruct(mat_t *file, const string &varName) {
  matvar_t *header = Mat_VarReadInfo(file, varName.c_str());

  if (header && header->class_type == MAT_C_STRUCT)
    return header;

  return nullptr;
}

matvar_t *readVar(mat_t *file, const std::string &varName, matvar_t **toFree) {
  auto nameParts = splitVarName(varName);
  matvar_t *var = nullptr;

  if (nameParts.second.empty()) {
    var = Mat_VarReadInfo(file, nameParts.first.c_str());

    *toFree = var;
  } else {
    matvar_t *matStruct = readStruct(file, nameParts.first);

    if (matStruct)
      var = Mat_VarGetStructFieldByName(matStruct, nameParts.second.c_str(), 0);

    *toFree = matStruct;
  }

  return var;
}

void readDataAll(mat_t *file, matvar_t *var) {
  int err = Mat_VarReadDataAll(file, var);
  assert(err == 0);
  (void)err;
}

vector<double> readDoubleArray(mat_t *file, matvar_t *var) {
  readDataAll(file, var);

  int cols = var->rank < 2 ? 1 : static_cast<int>(var->dims[1]);
  int size = cols * static_cast<int>(var->dims[0]);

  vector<double> doubleArray(size);
  decodeArray(var->data, doubleArray.data(), var->data_type, size);

  return doubleArray;
}

} // namespace

namespace AlenkaFile {

// How to decode data in Matlab: data = double(d)*diag(mults);

MAT::MAT(const string &filePath, MATvars vars)
    : DataFile(filePath), vars(std::move(vars)) {
  openMatFile(filePath);
  construct();
}

MAT::MAT(const std::vector<string> &filePaths, MATvars vars)
    : DataFile(filePaths.at(0)), vars(std::move(vars)) {
  for (auto e : filePaths)
    openMatFile(e);
  construct();
}

MAT::~MAT() {
  for (auto e : dataToFree)
    Mat_VarFree(e);

  for (auto e : files)
    Mat_Close(e);
}

void MAT::save() { DataFile::save(); }

bool MAT::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    loadEvents();
    return false;
  }

  return true;
}

void MAT::openMatFile(const string &filePath) {
  mat_t *file = Mat_Open(filePath.c_str(), MAT_ACC_RDONLY);

  if (!file)
    throwDetailed(runtime_error("Error while opening " + filePath));

  files.push_back(file);
}

void MAT::construct() {
  readSamplingRate();
  readData();
  readMults();
  readDate();
  readLabels();
}

void MAT::readSamplingRate() {
  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *fs = readVar(file, vars.frequency, &toFree);

    if (fs) {
      if (fs->dims[0] <= 0)
        throwDetailed(runtime_error("Bad MAT file format"));

      readDataAll(file, fs);
      decodeArray(fs->data, &samplingFrequency, fs->data_type);
    }

    Mat_VarFree(toFree);

    if (fs)
      return;
  }

  samplingFrequency = 1000;

  cerr << "Warning: var " << vars.frequency
       << " missing in MAT files; using default value=" << samplingFrequency
       << endl;
}

void MAT::readData() {
  numberOfChannels = MAX_CHANNELS;
  samplesRecorded = 0;

  for (auto varName : vars.data) {
    for (unsigned int i = 0; i < files.size(); ++i) {
      matvar_t *toFree;
      matvar_t *dataVar = readVar(files[i], varName, &toFree);

      if (dataVar) {
        if (dataVar->rank != 2)
          throwDetailed(
              runtime_error("Data var in MAT files must have rank 2"));

        sizes.push_back(static_cast<int>(dataVar->dims[0]));
        samplesRecorded += dataVar->dims[0];

        if (numberOfChannels == MAX_CHANNELS) {
          numberOfChannels = static_cast<int>(dataVar->dims[1]);

          if (MAX_CHANNELS <= numberOfChannels)
            throwDetailed(
                runtime_error("Too many channes in '" + varName +
                              "'. You probably saved the data with channels "
                              "in rows by mistake."));
        }

        if (numberOfChannels != static_cast<int>(dataVar->dims[1]))
          throwDetailed(runtime_error(
              "All data variables must have the same number of channels"));

        data.push_back(dataVar);
        dataToFree.push_back(toFree);
        dataFileIndex.push_back(i);
      }
    }
  }

  if (data.empty())
    throwDetailed(runtime_error("No data variables in MAT-files found"));
}

void MAT::readMults() {
  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *mults = readVar(file, vars.multipliers, &toFree);

    if (mults) {
      multipliers = readDoubleArray(file, mults);

      if (static_cast<int>(multipliers.size()) < numberOfChannels)
        throwDetailed(runtime_error("Bad MAT file format"));

      multipliers.resize(numberOfChannels);
    }

    Mat_VarFree(toFree);

    if (mults)
      return;
  }
}

void MAT::readDate() {
  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *date = readVar(file, vars.date, &toFree);

    if (date) {
      readDataAll(file, date);
      decodeArray(date->data, &days, date->data_type);
    }

    Mat_VarFree(toFree);

    if (date)
      return;
  }
}

void MAT::readLabels() {
  labels.resize(numberOfChannels, "");

  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *label = readVar(file, vars.label, &toFree);

    if (label && label->class_type == MAT_C_CELL) {
      for (int i = 0; i < numberOfChannels; ++i) {
        matvar_t *cell = Mat_VarGetCell(label, i);

        if (cell && cell->class_type == MAT_C_CHAR && cell->rank == 2 &&
            cell->dims[0] == 1) {
          readDataAll(file, cell);

          int dim1 = static_cast<int>(cell->dims[1]);
          char *dataPtr = reinterpret_cast<char *>(cell->data);

          for (int j = 0; j < dim1; ++j)
            labels[i].push_back(dataPtr[j]);
        }
      }
    }

    Mat_VarFree(toFree);

    if (label)
      break;
  }
}

void MAT::readEvents(vector<int> *eventPositions, vector<int> *eventDurations,
                     vector<int> *eventChannels) {
  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *pos = readVar(file, vars.eventPosition, &toFree);

    if (pos) {
      vector<double> doubleArray = readDoubleArray(file, pos);

      for (double i : doubleArray)
        eventPositions->push_back(
            static_cast<int>(round(i * samplingFrequency)));

      Mat_VarFree(toFree);
    }
  }

  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *dur = readVar(file, vars.eventDuration, &toFree);

    if (dur) {
      vector<double> doubleArray = readDoubleArray(file, dur);

      for (double i : doubleArray)
        eventDurations->push_back(
            static_cast<int>(round(i * samplingFrequency)));

      Mat_VarFree(toFree);
    }
  }

  for (mat_t *file : files) {
    matvar_t *toFree;
    matvar_t *chan = readVar(file, vars.eventChannel, &toFree);

    if (chan) {
      vector<double> doubleArray = readDoubleArray(file, chan);

      for (double i : doubleArray)
        eventChannels->push_back(static_cast<int>(round(i)) - 1);

      Mat_VarFree(toFree);
    }
  }

  size_t size = eventPositions->size();
  eventDurations->resize(size, 0);
  eventChannels->resize(size, -2);
}

template <typename T>
void MAT::readChannelsFloatDouble(vector<T *> dataChannels,
                                  uint64_t firstSample, uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("MAT: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("MAT: too few dataChannels");

  int i = 0;
  uint64_t lastInChunk = 0;

  while (lastInChunk += sizes[i], lastInChunk < firstSample)
    ++i;

  uint64_t firstInChunk = lastInChunk - sizes[i];
  --lastInChunk;

  for (uint64_t j = firstSample; j < lastSample;) {
    uint64_t last = min(lastSample, lastInChunk);
    int length = static_cast<int>(last - j + 1);
    tmpBuffer.resize(numberOfChannels * length * 8);

    int start[2] = {static_cast<int>(j - firstInChunk), 0};
    int stride[2] = {1, 1};
    int edge[2] = {length, numberOfChannels};

    int err = Mat_VarReadData(files[dataFileIndex[i]], data[i],
                              tmpBuffer.data(), start, stride, edge);
    assert(err == 0);
    (void)err;

    for (int k = 0; k < numberOfChannels; ++k) {
      decodeArray(tmpBuffer.data(), dataChannels[k], data[i]->data_type, length,
                  k * length);

      if (!multipliers.empty()) {
        T multi = static_cast<T>(multipliers[k]);

        for (int l = 0; l < length; ++l)
          dataChannels[k][l] *= multi;
      }
    }

    for (auto &e : dataChannels)
      e += length;
    firstInChunk += sizes[i];
    lastInChunk += sizes[i];
    j += length;
    ++i;
  }
}

void MAT::loadEvents() {
  vector<int> eventPositions;
  vector<int> eventDurations;
  vector<int> eventChannels;
  readEvents(&eventPositions, &eventDurations, &eventChannels);

  AbstractEventTable *eventTable =
      getDataModel()->montageTable()->eventTable(0);

  assert(eventPositions.size() == eventDurations.size() &&
         eventPositions.size() == eventChannels.size());
  int count = static_cast<int>(eventPositions.size());

  if (count > 0) {
    AbstractEventTypeTable *ett = getDataModel()->eventTypeTable();
    ett->insertRows(0);

    EventType et = ett->row(0);
    et.name = "MAT events";
    ett->row(0, et);

    eventTable->insertRows(0, count);

    for (int i = 0; i < count; ++i) {
      Event e = eventTable->row(i);

      e.type = 0;
      e.position = eventPositions[i];
      e.duration = eventDurations[i];
      e.channel = eventChannels[i];

      eventTable->row(i, e);
    }
  }
}

} // namespace AlenkaFile
