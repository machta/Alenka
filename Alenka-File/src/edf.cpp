#include "../include/AlenkaFile/edf.h"

#include "edflib_extended.h"
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <iostream>
#include <set>
#include <sstream>

#include <detailedexception.h>

using namespace std;
using namespace AlenkaFile;
using namespace boost;

namespace {

const int MIN_READ_CHUNK = 200;
const int OPT_READ_CHUNK = 2 * 1000;
const int MAX_READ_CHUNK = 2 * 1000 * 1000;
const double ZERO_TOLERANCE = 0.001;

double calculateDiffToAdd(double maxVal, double minVal) {
  double absMax = max(fabs(maxVal), fabs(minVal));
  double diff = fabs(maxVal - minVal) / absMax;

  if (absMax < ZERO_TOLERANCE || diff < ZERO_TOLERANCE)
    return 10;

  return 0;
}

void writeSignalInfo(int file, DataFile *dataFile,
                     const edf_hdr_struct *edfhdr) {
  int res = 0;
  auto labels = dataFile->getLabels();

  for (unsigned int i = 0; i < dataFile->getChannelCount(); ++i) {
    res |= edf_set_samplefrequency(
        file, i, static_cast<int>(dataFile->getSamplingFrequency()));

    double physMax = dataFile->getPhysicalMaximum(i);
    double physMin = dataFile->getPhysicalMinimum(i);
    // If the values are very close, add some so that the min and max is not the
    // same.
    double diff = calculateDiffToAdd(physMax, physMin);
    physMax += diff;
    physMin -= diff;
    res |= edf_set_physical_maximum(file, i, physMax);
    res |= edf_set_physical_minimum(file, i, physMin);

    res |= edf_set_digital_maximum(
        file, i,
        static_cast<int>(min<double>(dataFile->getDigitalMaximum(i), 32767)));
    res |= edf_set_digital_minimum(
        file, i,
        static_cast<int>(max<double>(dataFile->getDigitalMinimum(i), -32768)));
    res |= edf_set_label(file, i, labels[i].c_str());

    if (edfhdr) {
      res |= edf_set_prefilter(file, i, edfhdr->signalparam[i].prefilter);
      res |= edf_set_transducer(file, i, edfhdr->signalparam[i].transducer);
      res |= edf_set_physical_dimension(file, i,
                                        edfhdr->signalparam[i].physdimension);
    }
  }

  if (res)
    cerr << "Warning: EDF bad values in writeSignalInfo" << endl;
  assert(res == 0);
}

void writeMetaInfo(int file, const edf_hdr_struct *edfhdr) {
  if (!edfhdr)
    return;

  int res = 0;

  res |= edf_set_startdatetime(file, edfhdr->startdate_year,
                               edfhdr->startdate_month, edfhdr->startdate_day,
                               edfhdr->starttime_hour, edfhdr->starttime_minute,
                               edfhdr->starttime_second);
  res |= edf_set_patientname(file, edfhdr->patient_name);
  res |= edf_set_patientcode(file, edfhdr->patientcode);

  // TODO: Figure out how to set these. This version causes problems. Or don't
  // copy it at all: instead show a warning that the file will have some
  // incorrect values, and make it a known issue.
  // edf_set_gender_char(file, edfhdr->gender);
  // edf_set_birthdate_char(file, edfhdr->birthdate);

  res |= edf_set_patient_additional(file, edfhdr->patient_additional);
  res |= edf_set_admincode(file, edfhdr->admincode);
  res |= edf_set_technician(file, edfhdr->technician);
  res |= edf_set_equipment(file, edfhdr->equipment);
  res |= edf_set_recording_additional(file, edfhdr->recording_additional);

  if (res)
    cerr << "Warning: EDF bad values in writeMetaInfo" << endl;
  assert(res == 0);
}

long long convertEventPosition(int sample, double samplingFrequency) {
  return static_cast<long long>(round(sample / samplingFrequency * 10000));
}

int convertEventPositionBack(long long onset, double samplingFrequency) {
  return static_cast<int>(
      round(static_cast<double>(onset) * samplingFrequency / 10000 / 1000));
}

void saveAsWithType(const string &filePath, DataFile *sourceFile,
                    const edf_hdr_struct *edfhdr) {
  int numberOfChannels = sourceFile->getChannelCount();
  double samplingFrequency = sourceFile->getSamplingFrequency();
  uint64_t samplesRecorded = sourceFile->getSamplesRecorded();

  int type = EDFLIB_FILETYPE_EDFPLUS;
  if (edfhdr)
    type = edfhdr->filetype;
  if (type == EDFLIB_FILETYPE_EDF || type == EDFLIB_FILETYPE_BDF)
    ++type; // This is because edfopen_file_writeonly accepts only these two
            // types.

  int tmpFile =
      edfopen_file_writeonly(filePath.c_str(), type, numberOfChannels);

  if (tmpFile < 0)
    throwDetailed(
        runtime_error("edfopen_file_writeonly error: " + to_string(tmpFile)));

  // Copy data into the new file.
  writeSignalInfo(tmpFile, sourceFile, edfhdr);
  writeMetaInfo(tmpFile, edfhdr); // TODO: Write some of this (at least date)
                                  // even when saving as/exporting.

  int fs = static_cast<int>(round(samplingFrequency));
  unique_ptr<double[]> buffer(
      new double[numberOfChannels * fs * sizeof(double)]);

  for (uint64_t sampleIndex = 0; sampleIndex < samplesRecorded;
       sampleIndex += fs) {
    sourceFile->readSignal(buffer.get(), sampleIndex, sampleIndex + fs - 1);

    for (int i = 0; i < numberOfChannels; ++i) {
      int res = edfwrite_physical_samples(tmpFile, buffer.get() + i * fs);

      if (res != 0)
        throwDetailed(runtime_error("edfwrite_physical_samples failed"));
    }
  }

  // Write events.
  AbstractMontageTable *montageTable =
      sourceFile->getDataModel()->montageTable();

  for (int i = 0; i < montageTable->rowCount(); ++i) {
    if (montageTable->row(i).save) {
      AbstractEventTable *eventTable = montageTable->eventTable(i);

      for (int j = 0; j < eventTable->rowCount(); ++j) {
        Event e = eventTable->row(j);

        // Skip events belonging to tracks greater thatn the number of channels
        // in the file.
        // TODO: Perhaps make a warning about this?
        if (-1 <= e.channel && e.channel < static_cast<int>(numberOfChannels) &&
            e.type >= 0) {
          long long onset = convertEventPosition(e.position, samplingFrequency);
          long long duration =
              convertEventPosition(e.duration, samplingFrequency);

          stringstream ss;
          ss << "t=" << e.type << " c=" << e.channel << "|" << e.label << "|"
             << e.description;

          int res = edfwrite_annotation_utf8(tmpFile, onset, duration,
                                             ss.str().c_str());

          if (res != 0)
            throwDetailed(runtime_error("edfwrite_annotation_utf8 failed"));
        }
      }
    }
  }

  // Close the open files.
  int res = edfclose_file(tmpFile);

  if (res != 0)
    throwDetailed(runtime_error("Closing tmp EDF file failed"));
}

} // namespace

namespace AlenkaFile {

EDF::EDF(const string &filePath)
    : DataFile(filePath), edfhdr(new edf_hdr_struct) {
  openFile();
}

EDF::~EDF() {
  // TODO: Perhaps move this to the unique_ptr deleter.
  int err = edfclose_file(edfhdr->handle);
  if (err < 0)
    cerr << "Error closing EDF file" << endl;
}

double EDF::getStartDate() const {
  tm time;

  time.tm_mday = edfhdr->startdate_day;
  time.tm_mon = edfhdr->startdate_month - 1;
  time.tm_year = edfhdr->startdate_year - 1900;

  time.tm_sec = edfhdr->starttime_second;
  time.tm_min = edfhdr->starttime_minute;
  time.tm_hour = edfhdr->starttime_hour;

  return static_cast<double>(mktime(&time)) / 24 / 60 / 60 + daysUpTo1970;
}

// TODO: Allow tracking through a progress dialog as this can take a long time
// (because it has to recreate the whole file from scratch).
void EDF::save() {
  saveSecondaryFile();

  AbstractMontageTable *montageTable = getDataModel()->montageTable();
  int tablesToSave = 0;

  for (int i = 0; i < montageTable->rowCount(); ++i) {
    if (montageTable->row(i).save)
      ++tablesToSave;
  }

  if (tablesToSave == 0)
    return;

  // Make the new file under a temporary name.
  filesystem::path tmpPath =
      filesystem::unique_path(getFilePath() + ".%%%%.tmp");
  saveAsWithType(tmpPath.string(), this, edfhdr.get());

  // Save a backup of the original file.
  int res = edfclose_file(edfhdr->handle);
  assert(res == 0 && "EDF file couldn't be closed.");
  (void)res;

  filesystem::path backupPath = getFilePath() + ".backup";
  if (!filesystem::exists(backupPath))
    filesystem::rename(getFilePath(), backupPath);

  // Replace the original file with the new one.
  filesystem::rename(tmpPath, getFilePath());

  openFile();
}

bool EDF::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    loadEvents();
    return false;
  }

  return true;
}

double EDF::getPhysicalMaximum(unsigned int channel) {
  if (channel < getChannelCount())
    return edfhdr->signalparam[channel].phys_max;
  return 0;
}

double EDF::getPhysicalMinimum(unsigned int channel) {
  if (channel < getChannelCount())
    return edfhdr->signalparam[channel].phys_min;
  return 0;
}

double EDF::getDigitalMaximum(unsigned int channel) {
  if (channel < getChannelCount())
    return edfhdr->signalparam[channel].dig_max;
  return 0;
}

double EDF::getDigitalMinimum(unsigned int channel) {
  if (channel < getChannelCount())
    return edfhdr->signalparam[channel].dig_min;
  return 0;
}

string EDF::getLabel(unsigned int channel) {
  if (channel < getChannelCount())
    return edfhdr->signalparam[channel].label;
  return "";
}

void EDF::saveAs(const string &filePath, DataFile *sourceFile) {
  saveAsWithType(filePath, sourceFile, nullptr);
}

template <typename T>
void EDF::readChannelsFloatDouble(vector<T *> dataChannels,
                                  uint64_t firstSample, uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");
  assert(readChunk > 0);

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("EDF: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("EDF: too few dataChannels");

  int handle = edfhdr->handle;
  long long err;

  for (unsigned int i = 0; i < getChannelCount(); ++i) {
    err = edfseek(handle, i, firstSample, EDFSEEK_SET);

    if (err != static_cast<long long>(firstSample))
      throwDetailed(runtime_error("edfseek failed"));
  }

  uint64_t nextBoundary = (firstSample + readChunk - 1 / readChunk);

  while (firstSample <= lastSample) {
    uint64_t last = min(nextBoundary, lastSample + 1);
    int n = static_cast<int>(last - firstSample);

    for (unsigned int i = 0; i < getChannelCount(); ++i) {
      err = edfread_physical_samples(handle, i, n, readChunkBuffer.data());

      if (err != n)
        throwDetailed(runtime_error("edfread_physical_samples failed"));

      for (int j = 0; j < n; ++j)
        dataChannels[i][j] = static_cast<T>(readChunkBuffer[j]);

      dataChannels[i] += n;
    }

    firstSample += n;
    nextBoundary += readChunk;
  }
}

void EDF::openFile() {
  int err = edfopen_file_readonly(getFilePath().c_str(), edfhdr.get(),
                                  EDFLIB_READ_ALL_ANNOTATIONS);

  if (err < 0) {
    if (edfhdr->filetype == EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)
      throwDetailed(runtime_error("Warning: EDF format error"));
    else
      throwDetailed(runtime_error("edfopen_file_readonly error: " +
                                  to_string(edfhdr->filetype)));
  }

  samplesRecorded = edfhdr->signalparam[0].smp_in_file;

  samplingFrequency = static_cast<double>(samplesRecorded);
  samplingFrequency /= static_cast<double>(edfhdr->file_duration);
  samplingFrequency *= 10 * 1000 * 1000;

  numberOfChannels = edfhdr->edfsignals;

  readChunk = edfhdr->signalparam[0].smp_in_datarecord;
  if (readChunk < MIN_READ_CHUNK || readChunk > MAX_READ_CHUNK)
    readChunk = OPT_READ_CHUNK;
  else if (readChunk < OPT_READ_CHUNK)
    readChunk = OPT_READ_CHUNK - OPT_READ_CHUNK % readChunk;
  readChunkBuffer.resize(readChunk);
}

void EDF::loadEvents() {
  assert(0 < getDataModel()->montageTable()->rowCount());

  edf_annotation_struct event;
  int eventCount = static_cast<int>(edfhdr->annotations_in_file);

  AbstractEventTable *et = getDataModel()->montageTable()->eventTable(0);
  assert(et->rowCount() == 0);
  et->insertRows(0, eventCount);

  for (int i = 0; i < eventCount; ++i) {
    edf_get_annotation(edfhdr->handle, i, &event);

    Event e = et->row(i);

    e.position = convertEventPositionBack(event.onset, samplingFrequency);

    double duration = 0;
    sscanf(event.duration, "%lf", &duration);
    if (duration != 0)
      e.duration = static_cast<int>(round(duration * samplingFrequency));

    sscanf(event.annotation, "t=%d c=%d", &e.type, &e.channel);
    string annotation = event.annotation;

    auto first = annotation.find("|") + 1;
    auto second = annotation.find("|", first);

    string label = annotation.substr(first, second - first);
    if (label.size() > 0)
      e.label = label;

    string description = annotation.substr(second + 1);
    if (description.size() > 0)
      e.description = description;

    et->row(i, e);
  }

  addUsedEventTypes();
}

void EDF::addUsedEventTypes() {
  AbstractEventTypeTable *ett = getDataModel()->eventTypeTable();
  AbstractEventTable *et = getDataModel()->montageTable()->eventTable(0);

  set<int> typesUsed;
  for (int i = 0; i < et->rowCount(); ++i)
    typesUsed.insert(et->row(i).type);

  int count = static_cast<int>(typesUsed.size());
  assert(ett->rowCount() == 0);
  ett->insertRows(0, count);

  auto it = typesUsed.begin();
  for (int i = 0; i < count; ++i) {
    EventType et = ett->row(i);

    int id = *it;
    et.id = id;
    et.name = "Type " + to_string(id);

    ett->row(i, et);

    ++it;
  }

  for (int i = 0; i < et->rowCount(); ++i) {
    Event e = et->row(i);

    auto it = typesUsed.find(e.type);
    assert(it != typesUsed.end());

    e.type = static_cast<int>(distance(typesUsed.begin(), it));

    et->row(i, e);
  }
}

} // namespace AlenkaFile
