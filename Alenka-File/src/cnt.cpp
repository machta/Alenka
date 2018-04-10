#include "../include/AlenkaFile/cnt.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <detailedexception.h>

#include "cnt_header.h"

using namespace std;
using namespace AlenkaFile;

struct CntHeaderWrapper {
  FIXED_HEADER hdr;
  vector<ELECTLOC> channelHdr;
};

static_assert(900 == sizeof(FIXED_HEADER), "Check CNT header size");
static_assert(75 == sizeof(ELECTLOC), "Check CNT channel struct");

namespace {} // namespace

namespace AlenkaFile {

CNT::CNT(const string &filePath) : DataFile(filePath) {
  fileHeader = make_unique<CntHeaderWrapper>();
  openFile();
}

CNT::~CNT() {}

double CNT::getStartDate() const {
  // TODO
  return 0;
}

void CNT::save() {
  saveSecondaryFile();

  // Add saving.
}

bool CNT::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    // Add loading.

    return false;
  }

  return true;
}

// double CNT::getPhysicalMaximum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].PhysMax;
//  return 0;
//}

// double CNT::getPhysicalMinimum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].PhysMin;
//  return 0;
//}

// double CNT::getDigitalMaximum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].DigMax;
//  return 0;
//}

// double CNT::getDigitalMinimum(const unsigned int channel) {
//  if (channel < getChannelCount())
//    return fileHeader->hdr->CHANNEL[channel].DigMin;
//  return 0;
//}

string CNT::getLabel(const unsigned int channel) {
  if (channel < getChannelCount()) {
  }
  return "";
}

template <typename T>
void CNT::readChannelsFloatDouble(vector<T *> dataChannels,
                                  const uint64_t firstSample,
                                  const uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("BioSigFile: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("BioSigFile: too few dataChannels");

  file.seekg(dataOffset + sizeof(int16_t) * firstSample * numberOfChannels);
  auto rawBuffer = reinterpret_cast<char *>(sampleBuffer.data());

  for (uint64_t sampleIndex = firstSample; sampleIndex <= lastSample;
       ++sampleIndex) {
    file.read(rawBuffer, sizeof(int16_t) * numberOfChannels);

    for (int i = 0; i < numberOfChannels; ++i) {
      dataChannels[i][0] =
          (static_cast<float>(sampleBuffer[i]) - scalingFactors[i].first) *
          scalingFactors[i].second;
      dataChannels[i]++;
    }
  }
}

void CNT::openFile() {
  file = ifstream(getFilePath(), file.in | file.out | file.binary);

  if (!file.is_open())
    throwDetailed(runtime_error("Failed to open file"));

  auto rawHeader = reinterpret_cast<char *>(&fileHeader->hdr);
  file.read(rawHeader, sizeof(fileHeader->hdr));

  if (!file.good())
    throwDetailed(runtime_error("Failed read header"));

  samplingFrequency = fileHeader->hdr.rate / 100;
  numberOfChannels = fileHeader->hdr.nchannels;
  samplesRecorded = fileHeader->hdr.NumSamples;
  dataOffset = sizeof(FIXED_HEADER) + sizeof(ELECTLOC) * numberOfChannels;

  fileHeader->channelHdr.resize(numberOfChannels);
  auto rawChannelHeader =
      reinterpret_cast<char *>(fileHeader->channelHdr.data());
  file.read(rawChannelHeader, sizeof(ELECTLOC) * numberOfChannels);

  if (!file.good())
    throwDetailed(runtime_error("Failed read channel header"));
  assert(file.tellg() == dataOffset);
  file.exceptions(ifstream::failbit | ifstream::badbit);

  sampleBuffer.resize(numberOfChannels);
  scalingFactors.resize(numberOfChannels);

  for (int i = 0; i < numberOfChannels; ++i) {
    const float dc = fileHeader->channelHdr[i].baseline;
    const float sf = fileHeader->channelHdr[i].sensitivity *
                     fileHeader->channelHdr[i].calib / 204.8f * 1000 * 1000;
    scalingFactors[i] = {dc, sf};
  }
}

} // namespace AlenkaFile
