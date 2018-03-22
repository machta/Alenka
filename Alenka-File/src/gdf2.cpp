#include "../include/AlenkaFile/gdf2.h"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <set>
#include <stdexcept>

#include <detailedexception.h>

using namespace std;
using namespace AlenkaFile;
using namespace boost;

namespace {

const bool isLittleEndian = DataFile::testLittleEndian();

template <typename T>
void readFile(fstream &file, T *val, size_t elements = 1) {
  file.read(reinterpret_cast<char *>(val), sizeof(T) * elements);

  assert(file && "File read successfully.");
  assert(static_cast<size_t>(file.gcount()) == sizeof(T) * elements &&
         "Not all bytes were read from the file.");

  if (isLittleEndian == false) {
    for (unsigned int i = 0; i < elements; ++i)
      DataFile::changeEndianness(val + i);
  }
}

void seekFile(fstream &file, int64_t offset, bool fromStart = false,
              bool isGet = true) {
  if (isGet) {
    if (fromStart)
      file.seekg(offset);
    else
      file.seekg(offset, file.cur);
  } else {
    if (fromStart)
      file.seekp(offset);
    else
      file.seekp(offset, file.cur);
  }
}

template <typename T>
void writeFile(fstream &file, const T *val, unsigned int elements = 1) {
  if (isLittleEndian == false) {
    for (unsigned int i = 0; i < elements; ++i) {
      T tmp = val[i];
      DataFile::changeEndianness(&tmp);

      file.write(reinterpret_cast<char *>(&tmp), sizeof(T));
    }
  } else {
    file.write(reinterpret_cast<const char *>(val), sizeof(T) * elements);
  }
}

#ifndef NDEBUG
streampos tellFile(fstream &file, bool isGet = true) {
  return isGet ? file.tellg() : file.tellp();
}
#endif

template <class T> void convertSamples(T *dataBuffer, double *samples, int n) {
  for (int i = 0; i < n; ++i) {
    samples[i] = static_cast<double>(dataBuffer[i]);
  }
}

void readRecord(fstream &file, char *rawBuffer, double *samples, int n,
                int sampleSize, int dataType, bool isLittleEndian) {
  file.read(rawBuffer, sampleSize * n);

  if (isLittleEndian == false) {
    for (int i = 0; i < n; ++i)
      DataFile::changeEndianness(rawBuffer + i * sampleSize, sampleSize);
  }

#define CASE(a_, b_)                                                           \
  case a_:                                                                     \
    convertSamples(reinterpret_cast<b_ *>(rawBuffer), samples, n);             \
    break;

  switch (dataType) {
    CASE(1, int8_t);
    CASE(2, uint8_t);
    CASE(3, int16_t);
    CASE(4, uint16_t);
    CASE(5, int32_t);
    CASE(6, uint32_t);
    CASE(7, int64_t);
    CASE(8, uint64_t);
    CASE(16, float);
    CASE(17, double);
  default:
    assert(0);
  }

#undef CASE
}

/**
 * @brief calibrateSamples
 *
 * This must be used in order to scale the samples properly.
 * The samples are usually stored as ineger values and must be "adjusted"
 * to get the proper floating point value.
 *
 * When this is not used, the raw data is returned.
 */
void calibrateSamples(double *samples, int n, double digitalMinimum,
                      double scale, double physicalMinimum) {
  for (int i = 0; i < n; ++i) {
    samples[i] -= digitalMinimum;
    samples[i] /= scale;
    samples[i] += physicalMinimum;
  }
}

} // namespace

// TODO: handle fstream exceptions in a clear and more informative way

namespace AlenkaFile {

GDF2::GDF2(const string &filePath, bool uncalibrated) : DataFile(filePath) {
  file.open(filePath, file.in | file.out | file.binary);

  if (!file.is_open())
    throwDetailed(runtime_error("GDF2 file could not be successfully opened"));

  file.exceptions(ifstream::failbit | ifstream::badbit);

  // Load fixed header.
  seekFile(file, 0, true);

  readFile(file, fh.versionID, 8);
  fh.versionID[8] = 0;
  int major, minor;
  sscanf(fh.versionID + 4, "%d.%d", &major, &minor);
  version = minor + 100 * major;

  if (string(fh.versionID, 3) != "GDF" || major != 2)
    throwDetailed(runtime_error("Unrecognized file format."));

  readFile(file, fh.patientID, 66);
  fh.patientID[66] = 0;

  seekFile(file, 10);

  readFile(file, &fh.drugs);

  readFile(file, &fh.weight);

  readFile(file, &fh.height);

  readFile(file, &fh.patientDetails);

  readFile(file, fh.recordingID, 64);
  fh.recordingID[64] = 0;

  readFile(file, fh.recordingLocation, 4);

  readFile(file, fh.startDate, 2);

  readFile(file, fh.birthday, 2);

  readFile(file, &fh.headerLength);

  readFile(file, fh.ICD, 6);

  readFile(file, &fh.equipmentProviderID);

  seekFile(file, 6);

  readFile(file, fh.headsize, 3);

  readFile(file, fh.positionRE, 3);

  readFile(file, fh.positionGE, 3);

  readFile(file, &fh.numberOfDataRecords);

  if (fh.numberOfDataRecords < 0)
    runtime_error(
        "GDF file with unknown number of data records is not supported.");

  double duration;
  if (version > 220) {
    double *ptr = reinterpret_cast<double *>(fh.durationOfDataRecord);
    readFile(file, ptr, 1);
    duration = *ptr;
  } else {
    uint32_t *ptr = reinterpret_cast<uint32_t *>(fh.durationOfDataRecord);
    readFile(file, ptr, 2);
    duration = static_cast<double>(ptr[0]) / static_cast<double>(ptr[1]);
  }

  readFile(file, &fh.numberOfChannels);

  // Load variable header.
  resizeVariableHeaderFields();

  seekFile(file, 2);
  assert(tellFile(file) == streampos(256) &&
         "Make sure we read all of the fixed header.");

  for (unsigned int i = 0; i < getChannelCount(); ++i) {
    readFile(file, vh.label[i].data(), vh.label[i].size() - 1);
    vh.label[i].back() = 0;
  }

  for (unsigned int i = 0; i < getChannelCount(); ++i) {
    readFile(file, vh.typeOfSensor[i].data(), vh.typeOfSensor[i].size() - 1);
    vh.typeOfSensor[i].back() = 0;
  }

  seekFile(file, 6 * getChannelCount());

  readFile(file, vh.physicalDimensionCode.data(), getChannelCount());
  readFile(file, vh.physicalMinimum.data(), getChannelCount());
  readFile(file, vh.physicalMaximum.data(), getChannelCount());
  readFile(file, vh.digitalMinimum.data(), getChannelCount());
  readFile(file, vh.digitalMaximum.data(), getChannelCount());

  seekFile(file, 64 * getChannelCount());

  readFile(file, vh.timeOffset.data(), getChannelCount());
  readFile(file, vh.lowpass.data(), getChannelCount());
  readFile(file, vh.highpass.data(), getChannelCount());
  readFile(file, vh.notch.data(), getChannelCount());
  readFile(file, vh.samplesPerRecord.data(), getChannelCount());
  readFile(file, vh.typeOfData.data(), getChannelCount());

  for (unsigned int i = 0; i < getChannelCount(); ++i)
    readFile(file, vh.sensorPosition[i].data(), vh.sensorPosition[i].size());
  for (unsigned int i = 0; i < getChannelCount(); ++i)
    readFile(file, vh.sensorInfo[i].data(), vh.sensorInfo[i].size());

  assert((tellFile(file) == streampos(-1) ||
          tellFile(file) == streampos(256 + 256 * getChannelCount())) &&
         "Make sure we read all of the variable header.");

  // Initialize other members.
  samplesRecorded = vh.samplesPerRecord[0] * fh.numberOfDataRecords;

  startOfData = 256 * fh.headerLength;

#define CASE(a_, b_)                                                           \
  case a_:                                                                     \
    dataTypeSize = sizeof(b_);                                                 \
    break;

  switch (dataType = vh.typeOfData[0]) {
    CASE(1, int8_t);
    CASE(2, uint8_t);
    CASE(3, int16_t);
    CASE(4, uint16_t);
    CASE(5, int32_t);
    CASE(6, uint32_t);
    CASE(7, int64_t);
    CASE(8, uint64_t);
    CASE(16, float);
    CASE(17, double);
  default:
    throwDetailed(runtime_error("Unsupported data type for GDF2"));
  }

#undef CASE

  if (uncalibrated == false) {
    scale.resize(getChannelCount());

    for (unsigned int i = 0; i < getChannelCount(); ++i) {
      scale[i] = (vh.digitalMaximum[i] - vh.digitalMinimum[i]) /
                 (vh.physicalMaximum[i] - vh.physicalMinimum[i]);
    }
  }

  samplingFrequency = vh.samplesPerRecord[0] / duration;

  int64_t dataRecordBytes =
      vh.samplesPerRecord[0] * getChannelCount() * dataTypeSize;
  startOfEventTable = startOfData + dataRecordBytes * fh.numberOfDataRecords;

  recordRawBuffer.resize(vh.samplesPerRecord[0] * dataTypeSize);
  recordDoubleBuffer.resize(vh.samplesPerRecord[0]);
}

void GDF2::save() {
  saveSecondaryFile();

  // Collect events from montages marked 'save'.
  vector<uint32_t> positions;
  vector<uint16_t> types;
  vector<uint16_t> channels;
  vector<uint32_t> durations;

  AbstractMontageTable *montageTable = getDataModel()->montageTable();

  for (int i = 0; i < montageTable->rowCount(); ++i) {
    if (montageTable->row(i).save) {
      AbstractEventTable *eventTable = montageTable->eventTable(i);

      for (int j = 0; j < eventTable->rowCount(); ++j) {
        Event e = eventTable->row(j);

        // Skip events belonging to tracks greater thatn the number of channels
        // in the file. TODO: Perhaps make a warning about this?
        if (-1 <= e.channel &&
            e.channel < static_cast<int>(getChannelCount()) && e.type >= 0) {
          positions.push_back(e.position + 1);
          types.push_back(static_cast<uint16_t>(
              getDataModel()->eventTypeTable()->row(e.type).id));
          channels.push_back(
              static_cast<uint16_t>(e.channel + 1)); // TODO: Make a warning if
                                                     // these values cannot be
                                                     // converted properly.
          durations.push_back(e.duration);
        }
      }
    }
  }

  // Make a backup copy.
  filesystem::path backupPath = getFilePath() + ".backup";
  if (!filesystem::exists(backupPath))
    filesystem::copy(getFilePath(), backupPath);

  // Write mode, NEV and SR.
  seekFile(file, startOfEventTable, true);

  uint8_t eventTableMode = 3;
  writeFile(file, &eventTableMode);

  int numberOfEvents = min(
      static_cast<int>(positions.size()),
      (1 << 24) - 1); // 2^24 - 1 is the maximum length of the gdf event table
  uint8_t nev[3];
  int tmp = numberOfEvents;
  nev[0] = static_cast<uint8_t>(tmp % 256);
  tmp >>= 8;
  nev[1] = static_cast<uint8_t>(tmp % 256);
  tmp >>= 8;
  nev[2] = static_cast<uint8_t>(tmp % 256);
  if (isLittleEndian == false)
    changeEndianness(reinterpret_cast<char *>(nev), 3);
  writeFile(file, nev, 3);

  float sr = static_cast<float>(getSamplingFrequency());
  writeFile(file, &sr);

  // Write the events to the gdf event table.
  writeFile(file, positions.data(), numberOfEvents);
  writeFile(file, types.data(), numberOfEvents);
  writeFile(file, channels.data(), numberOfEvents);
  writeFile(file, durations.data(), numberOfEvents);

  file.sync();
}

bool GDF2::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    readGdfEventTable();
    return false;
  }

  return true;
}

void GDF2::resizeVariableHeaderFields() {
  auto count = getChannelCount();

  vh.label.resize(count);
  vh.typeOfSensor.resize(count);

  vh.physicalDimensionCode.resize(count);
  vh.physicalMinimum.resize(count);
  vh.physicalMaximum.resize(count);
  vh.digitalMinimum.resize(count);
  vh.digitalMaximum.resize(count);

  vh.timeOffset.resize(count);
  vh.lowpass.resize(count);
  vh.highpass.resize(count);
  vh.notch.resize(count);
  vh.samplesPerRecord.resize(count);
  vh.typeOfData.resize(count);
  vh.sensorPosition.resize(count);
  vh.sensorInfo.resize(count);
}

template <typename T>
void GDF2::readChannelsFloatDouble(vector<T *> dataChannels,
                                   const uint64_t firstSample,
                                   const uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("GDF2: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("GDF2: too few dataChannels");

  int samplesPerRecord = vh.samplesPerRecord[0];
  int recordChannelBytes = samplesPerRecord * dataTypeSize;

  uint64_t recordI = firstSample / samplesPerRecord;
  seekFile(file, startOfData + recordI * recordChannelBytes * getChannelCount(),
           true);

  int firstSampleToCopy = static_cast<int>(firstSample % samplesPerRecord);

  for (; recordI <= lastSample / samplesPerRecord; ++recordI) {
    int copyCount =
        min(samplesPerRecord - firstSampleToCopy,
            static_cast<int>(lastSample - recordI * samplesPerRecord) -
                firstSampleToCopy + 1);

    assert(copyCount > 0 && "Ensure there is something to copy");
    assert(firstSample + copyCount - 1 <= lastSample &&
           "Make sure we don't write beyond the output buffer.");
    assert(firstSampleToCopy + copyCount <= samplesPerRecord &&
           "Make sure we don't acceed tmp buffer size.");

    for (unsigned int channelI = 0; channelI < getChannelCount(); ++channelI) {
      readRecord(file, recordRawBuffer.data(), recordDoubleBuffer.data(),
                 samplesPerRecord, dataTypeSize, dataType, isLittleEndian);

      if (!scale.empty())
        calibrateSamples(recordDoubleBuffer.data(), samplesPerRecord,
                         vh.digitalMinimum[channelI], scale[channelI],
                         vh.physicalMinimum[channelI]);

      for (int i = 0; i < copyCount; ++i)
        dataChannels[channelI][i] =
            static_cast<T>(recordDoubleBuffer[firstSampleToCopy + i]);

      dataChannels[channelI] += copyCount;
    }

    firstSampleToCopy = 0;
  }
}

void GDF2::readGdfEventTable() {
  seekFile(file, startOfEventTable, true);

  uint8_t eventTableMode;
  readFile(file, &eventTableMode);

  uint8_t nev[3];
  readFile(file, nev, 3);
  if (isLittleEndian == false)
    changeEndianness(reinterpret_cast<char *>(nev), 3);
  int numberOfEvents = nev[0] + nev[1] * 256 + nev[2] * 256 * 256;

  seekFile(file, 4);

  if (numberOfEvents == 0)
    return;

  AbstractEventTable *defaultEvents =
      getDataModel()->montageTable()->eventTable(0);
  defaultEvents->insertRows(0, numberOfEvents);

  for (int i = 0; i < numberOfEvents; ++i) {
    uint32_t position;
    readFile(file, &position);

    Event e = defaultEvents->row(i);
    int tmp = position;
    e.position = tmp - 1;
    defaultEvents->row(i, e);
  }

  set<int> eventTypesUsed;

  for (int i = 0; i < numberOfEvents; ++i) {
    uint16_t type;
    readFile(file, &type);
    eventTypesUsed.insert(type);

    Event e = defaultEvents->row(i);
    e.type = type;
    defaultEvents->row(i, e);
  }

  for (int i = 0; i < numberOfEvents; ++i) {
    int type = defaultEvents->row(i).type;
    type = static_cast<int>(
        distance(eventTypesUsed.begin(), eventTypesUsed.find(type)));

    Event e = defaultEvents->row(i);
    e.type = type;
    defaultEvents->row(i, e);
  }

  if (eventTableMode & 0x02) {
    for (int i = 0; i < numberOfEvents; ++i) {
      uint16_t channel;
      readFile(file, &channel);
      int tmp = channel - 1;

      if (tmp >= static_cast<int>(getChannelCount()))
        tmp = -1;

      Event e = defaultEvents->row(i);
      e.channel = tmp;
      defaultEvents->row(i, e);
    }

    for (int i = 0; i < numberOfEvents; ++i) {
      uint32_t duration;
      readFile(file, &duration);

      Event e = defaultEvents->row(i);
      e.duration = duration;
      defaultEvents->row(i, e);
    }
  }

  // Add all event types used in the gdf event table.
  AbstractEventTypeTable *ett = getDataModel()->eventTypeTable();

  for (const auto &e : eventTypesUsed) {
    int row = ett->rowCount();
    ett->insertRows(row);

    EventType et = ett->row(row);
    et.id = e;
    et.name = "Type " + to_string(e);
    ett->row(row, et);
  }
}

} // namespace AlenkaFile
