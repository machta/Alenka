#include "../include/AlenkaFile/biosigfile.h"

//#include "../../libraries/biosig/biosig.h"
#include <biosig.h>

#include <algorithm>
#include <cassert>

#include <detailedexception.h>

using namespace std;
using namespace AlenkaFile;

struct BioSigHeaderWrapper {
  HDRTYPE *hdr;
};

namespace {

static_assert(sizeof(biosig_data_type) == sizeof(double),
              "Check float data type.");

} // namespace

namespace AlenkaFile {

BioSigFile::BioSigFile(const string &filePath) : DataFile(filePath) {
  openFile();
}

BioSigFile::~BioSigFile() {
  if (fileHeader)
    destructHDR(fileHeader->hdr);
}

void BioSigFile::save() {
  saveSecondaryFile();

  // Add saving.
}

bool BioSigFile::load() {
  if (DataFile::loadSecondaryFile() == false) {
    if (getDataModel()->montageTable()->rowCount() == 0)
      getDataModel()->montageTable()->insertRows(0);
    fillDefaultMontage(0);

    // Add loading.

    return false;
  }

  return true;
}

double BioSigFile::getPhysicalMaximum(const unsigned int channel) {
  if (channel < getChannelCount())
    return fileHeader->hdr->CHANNEL[channel].PhysMax;
  return 0;
}

double BioSigFile::getPhysicalMinimum(const unsigned int channel) {
  if (channel < getChannelCount())
    return fileHeader->hdr->CHANNEL[channel].PhysMin;
  return 0;
}

double BioSigFile::getDigitalMaximum(const unsigned int channel) {
  if (channel < getChannelCount())
    return fileHeader->hdr->CHANNEL[channel].DigMax;
  return 0;
}

double BioSigFile::getDigitalMinimum(const unsigned int channel) {
  if (channel < getChannelCount())
    return fileHeader->hdr->CHANNEL[channel].DigMin;
  return 0;
}

string BioSigFile::getLabel(const unsigned int channel) {
  if (channel < getChannelCount()) {
    return fileHeader->hdr->CHANNEL[channel].Label;
    // return biosig_channel_get_label(biosig_get_channel(fileHeader, channel));
  }
  return "";
}

template <typename T>
void BioSigFile::readChannelsFloatDouble(vector<T *> dataChannels,
                                         const uint64_t firstSample,
                                         const uint64_t lastSample) {
  assert(firstSample <= lastSample && "Bad parameter order.");
  assert(readChunk > 0);

  if (getSamplesRecorded() <= lastSample)
    invalid_argument("BioSigFile: reading out of bounds");
  if (dataChannels.size() < getChannelCount())
    invalid_argument("BioSigFile: too few dataChannels");

  size_t blockIndex = firstSample / readChunk;
  uint64_t nextSample = firstSample;

  while (nextSample <= lastSample) {
    const size_t blocksRead =
        sread(readChunkBuffer.data(), blockIndex, 1, fileHeader->hdr);
    assert(1 == blocksRead); // Add error handling

    const uint64_t lastSampleInBlock = (blockIndex + 1) * readChunk;
    const int n =
        min(lastSampleInBlock - nextSample, lastSample + 1 - nextSample);
    const int startOffset = nextSample - blockIndex * readChunk;

    for (unsigned int i = 0; i < getChannelCount(); ++i) {
      const int chunkRowStart = i * readChunk + startOffset;
      assert(readChunk >= startOffset + n);
      for (int j = 0; j < n; ++j)
        dataChannels[i][j] = readChunkBuffer[chunkRowStart + j];
      dataChannels[i] += n;
    }

    nextSample += n;
    ++blockIndex;
  }
}

void BioSigFile::openFile() {
  fileHeader = make_unique<BioSigHeaderWrapper>();
  fileHeader->hdr = sopen(getFilePath().c_str(), "r", nullptr);

  if (checkOpenFile()) {
    samplingFrequency = fileHeader->hdr->SampleRate;
    // samplingFrequency = biosig_get_samplerate(fileHeader);
    numberOfChannels = fileHeader->hdr->NS;
    // numberOfChannels = biosig_get_number_of_channels(fileHeader);
    samplesRecorded = fileHeader->hdr->SPR * fileHeader->hdr->NRec;
    // samplesRecorded = biosig_get_number_of_samples(fileHeader);

    readChunk = fileHeader->hdr->SPR;
    // readChunk = biosig_get_number_of_samples_per_record(fileHeader);
    readChunkBuffer.resize(readChunk * numberOfChannels);
  }
}

bool BioSigFile::checkOpenFile() {
  if (fileHeader->hdr && fileHeader->hdr->FILE.FID) {
    return true;
  }

  throwDetailed(runtime_error("failed to open file " + getFilePath()));
  return false;
}

} // namespace AlenkaFile
