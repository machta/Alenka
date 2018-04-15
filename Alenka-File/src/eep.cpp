#include "../include/AlenkaFile/eep.h"

extern "C" { // This must be here because it's a C header.
#include <v4/eep.h>
}

#include <cassert>
#include <memory>

#include <detailedexception.h>
#include <localeoverride.h>

using namespace std;
using namespace AlenkaFile;

namespace {

// This ensures we do clean up every time.
struct LibeepInitilizer {
  LibeepInitilizer() { libeep_init(); }
  ~LibeepInitilizer() { libeep_exit(); }
};
unique_ptr<LibeepInitilizer> libeep;

} // namespace

namespace AlenkaFile {

EEP::EEP(const string &filePath) : DataFile(filePath) {
  if (!libeep) // This makes sure we initialize only once.
    libeep = make_unique<LibeepInitilizer>();

  openFile();
}

EEP::~EEP() { libeep_close(fileHandle); }

std::time_t EEP::getStandardStartDate() const {
  return libeep_get_start_time(fileHandle);
}

void EEP::save() {
  saveSecondaryFile();

  // Add saving.
}

bool EEP::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    // Add loading.

    return false;
  }

  return true;
}

// double EEP::getPhysicalMaximum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].PhysMax;
//  return 0;
//}

// double EEP::getPhysicalMinimum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].PhysMin;
//  return 0;
//}

// double EEP::getDigitalMaximum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].DigMax;
//  return 0;
//}

// double EEP::getDigitalMinimum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].DigMin;
//  return 0;
//}

string EEP::getLabel(const unsigned int channel) {
  if (channel < getChannelCount())
    return libeep_get_channel_label(fileHandle, channel);
  return "";
}

template <typename T>
void EEP::readChannelsFloatDouble(vector<T *> dataChannels,
                                  const uint64_t firstSample,
                                  const uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("EEP: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("EEP: too few dataChannels");

  float *const sampleBuffer =
      libeep_get_samples(fileHandle, firstSample, lastSample + 1);
  if (nullptr == sampleBuffer)
    throwDetailed(std::runtime_error("libeep_get_samples() failed"));

  float *currentSample = sampleBuffer;
  for (uint64_t sampleIndex = firstSample; sampleIndex <= lastSample;
       ++sampleIndex) {
    for (int i = 0; i < numberOfChannels; ++i) {
      dataChannels[i][0] = *currentSample;
      currentSample++;
      dataChannels[i]++;
    }
  }

  libeep_free_samples(sampleBuffer);
}

void EEP::openFile() {
  LocaleOverride::executeWithCLocale(
      [this]() { fileHandle = libeep_read(getFilePath().c_str()); });
  if (-1 == fileHandle)
    throwDetailed(std::runtime_error("libeep_read() failed"));

  samplingFrequency = libeep_get_sample_frequency(fileHandle);
  numberOfChannels = libeep_get_channel_count(fileHandle);
  samplesRecorded = libeep_get_sample_count(fileHandle);
}

} // namespace AlenkaFile
