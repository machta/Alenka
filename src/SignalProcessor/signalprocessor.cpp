#include "signalprocessor.h"

#include "../../Alenka-File/include/AlenkaFile/datafile.h"
#include "../../Alenka-Signal/include/AlenkaSignal/filter.h"
#include "../../Alenka-Signal/include/AlenkaSignal/filterprocessor.h"
#include "../../Alenka-Signal/include/AlenkaSignal/montageprocessor.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"
#include "../DataModel/vitnessdatamodel.h"
#include "../myapplication.h"
#include "../options.h"

#include <QFile>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace AlenkaFile;

namespace {

const AbstractTrackTable *getTrackTable(OpenDataFile *file) {
  return file->dataModel->montageTable()->trackTable(
      OpenDataFile::infoTable.getSelectedMontage());
}

void multiplySamples(vector<float> *samples) {
  const vector<pair<double, double>> input =
      OpenDataFile::infoTable.getFrequencyMultipliers();

  if (input.empty())
    return;

  vector<float> multipliers(samples->size(), 1);

  for (auto e : input) {
    const int f = max<int>(0, round(e.first));
    const float multi = static_cast<float>(e.second);

    fill(multipliers.begin() + f, multipliers.end(), multi);
  }

  transform(samples->begin(), samples->end(), multipliers.begin(),
            samples->begin(), [](int a, int b) { return a * b; });
}

// TODO: Fix this this *allocator* hack. Change the cache so that it default
// constructs the elements as needed; then return pointers to these elements.
// Then there would be no need for an allocator -- the specialized allocation
// could be done by a simple object that owns the resources (a vector that holds
// an array, or a wrapper around an OpenGL buffer).
class FloatAllocator : public LRUCacheAllocator<float> {
  const int size;

public:
  FloatAllocator(int size) : size(size) {}

  bool constructElement(float **ptr) override {
    *ptr = new float[size];
    return true;
  }
  void destroyElement(float *ptr) override { delete[] ptr; }
};

} // namespace

SignalProcessor::SignalProcessor(unsigned int nBlock,
                                 unsigned int parallelQueues,
                                 int montageCopyCount,
                                 function<void()> glSharing, OpenDataFile *file,
                                 AlenkaSignal::OpenCLContext *context,
                                 int extraSamplesFront, int extraSamplesBack)
    : nBlock(nBlock), parallelQueues(parallelQueues),
      montageCopyCount(montageCopyCount), glSharing(std::move(glSharing)),
      file(file), context(context), extraSamplesFront(extraSamplesFront),
      extraSamplesBack(extraSamplesBack) {
  maxMontageTracks = programOption<int>("kernelCacheSize");

  fileChannels = file->file->getChannelCount();
  cl_int err;
  size_t size = (nBlock + 2) * fileChannels * sizeof(float);

  for (unsigned int i = 0; i < parallelQueues; ++i) {
    commandQueues.push_back(clCreateCommandQueue(
        context->getCLContext(), context->getCLDevice(), 0, &err));
    checkClErrorCode(err, "clCreateCommandQueue()");

    cl_mem_flags flags = CL_MEM_READ_WRITE;
    rawBuffers.push_back(
        clCreateBuffer(context->getCLContext(), flags, size, nullptr, &err));
    checkClErrorCode(err, "clCreateBuffer()");

#ifdef NDEBUG
    if (!programOption<bool>("cl11"))
      flags |= CL_MEM_HOST_NO_ACCESS;
#endif
    filterBuffers.push_back(
        clCreateBuffer(context->getCLContext(), flags, size, nullptr, &err));
    checkClErrorCode(err, "clCreateBuffer()");

    filterProcessors.push_back(
        make_unique<AlenkaSignal::FilterProcessor<float>>(nBlock, fileChannels,
                                                          context));
  }

  int blockFloats = nBlock * fileChannels;
  int64_t fileCacheMemory = programOption<int>("fileCacheSize");
  fileCacheMemory *= 1000 * 1000 / sizeof(float); // Convert from MB.
  int capacity = max(1, static_cast<int>(fileCacheMemory / blockFloats));

  logToFile("Creating File cache with " << capacity
                                        << " capacity and blocks of size "
                                        << blockFloats * sizeof(float) << ".");
  cache = make_unique<LRUCache<int, float>>(
      capacity, make_unique<FloatAllocator>(blockFloats));

  updateFilter();
  setUpdateMontageFlag();

  createXyzBuffer();
}

SignalProcessor::~SignalProcessor() {
  cl_int err;

  for (unsigned int i = 0; i < parallelQueues; ++i) {
    err = clReleaseCommandQueue(commandQueues[i]);
    checkClErrorCode(err, "clReleaseCommandQueue()");

    err = clReleaseMemObject(rawBuffers[i]);
    checkClErrorCode(err, "clReleaseMemObject()");

    err = clReleaseMemObject(filterBuffers[i]);
    checkClErrorCode(err, "clReleaseMemObject()");
  }

  if (xyzBuffer) {
    err = clReleaseMemObject(xyzBuffer);
    checkClErrorCode(err, "clReleaseMemObject()");
  }

  QObject::disconnect(xyzBufferConnection);
}

void SignalProcessor::updateFilter() {
  using namespace std;

  if (!file)
    return;

  M = file->file->getSamplingFrequency() + 1;

  filter = make_unique<AlenkaSignal::Filter<float>>(
      M, file->file->getSamplingFrequency());

  filter->setLowpassOn(OpenDataFile::infoTable.getLowpassOn());
  filter->setLowpass(OpenDataFile::infoTable.getLowpassFrequency());

  filter->setHighpassOn(OpenDataFile::infoTable.getHighpassOn());
  filter->setHighpass(OpenDataFile::infoTable.getHighpassFrequency());

  filter->setNotchOn(OpenDataFile::infoTable.getNotchOn());
  filter->setNotch(programOption<double>("notchFrequency"));

  auto samples = filter->computeSamples();
  if (OpenDataFile::infoTable.getFrequencyMultipliersOn())
    multiplySamples(&samples);

  for (unsigned int i = 0; i < parallelQueues; ++i) {
    filterProcessors[i]->changeSampleFilter(M, samples);
    filterProcessors[i]->applyWindow(OpenDataFile::infoTable.getFilterWindow());
  }

  nDiscard = filterProcessors[0]->discardSamples();
  nDelay = filterProcessors[0]->delaySamples();
  nMontage = nBlock - nDiscard;
  nSamples = nMontage - (extraSamplesFront + extraSamplesBack);

  OpenDataFile::infoTable.setFilterCoefficients(
      filterProcessors[0]->getCoefficients());
}

void SignalProcessor::setUpdateMontageFlag() {
  if (file) {
    trackCount = 0;

    if (0 < file->dataModel->montageTable()->rowCount()) {
      for (int i = 0; i < getTrackTable(file)->rowCount(); ++i) {
        const Track t = getTrackTable(file)->row(i);
        if (!t.hidden)
          ++trackCount;
      }
    }

    if (trackCount > 0)
      updateMontageFlag = true;
  }
}

void SignalProcessor::process(const vector<int> &indexVector,
                              const vector<cl_mem> &outBuffers) {
#ifndef NDEBUG
  assert(ready());
  assert(0 < indexVector.size());
  assert(static_cast<unsigned int>(indexVector.size()) <= parallelQueues);
  assert(indexVector.size() == outBuffers.size());

  for (unsigned int i = 0; i < outBuffers.size(); ++i)
    for (unsigned int j = 0; j < outBuffers.size(); ++j)
      assert(i == j || (outBuffers[i] != outBuffers[j] &&
                        indexVector[i] != indexVector[j]));
#endif

  if (updateMontageFlag) {
    updateMontageFlag = false;
    updateMontage();
  }

  cl_int err;
  const unsigned int iters =
      min(parallelQueues, static_cast<unsigned int>(indexVector.size()));

  for (unsigned int i = 0; i < iters; ++i) {
    // Load the signal data into the file cache.
    int index = indexVector[i], cacheIndex;

    float *fileBuffer = cache->getAny(set<int>{index}, &cacheIndex);
    assert(!fileBuffer || cacheIndex == index);

    if (!fileBuffer) {
      fileBuffer = cache->setOldest(index);
      logToFile("Loading block " << index << " to File cache.");

      auto fromTo = blockIndexToSampleRange(index, nSamples);
      fromTo.first += -nDiscard + nDelay - extraSamplesFront;
      fromTo.second += nDelay + extraSamplesBack;
      assert(fromTo.second - fromTo.first + 1 == nBlock);

      file->file->readSignal(fileBuffer, fromTo.first, fromTo.second);
    }

    assert(fileBuffer);
    printBuffer("after_readSignal.txt", fileBuffer, nBlock * fileChannels);

    size_t origin[] = {0, 0, 0};
    size_t rowLen = nBlock * sizeof(float);
    size_t region[] = {rowLen, fileChannels, 1};

    err = clEnqueueWriteBufferRect(
        commandQueues[i], rawBuffers[i], CL_TRUE, origin, origin, region,
        rowLen + 2 * sizeof(float), 0, 0, 0, fileBuffer, 0, nullptr, nullptr);
    checkClErrorCode(err, "clEnqueueWriteBufferRect()");

    if (!allpass()) {
      // Enqueu the filter operation, and store the result in the second buffer.
      printBuffer("before_filter.txt", rawBuffers[i], commandQueues[i]);
      filterProcessors[i]->process(rawBuffers[i], filterBuffers[i],
                                   commandQueues[i]);
      printBuffer("after_filter.txt", filterBuffers[i], commandQueues[i]);
    }
  }

  // TODO: Right here would be a great place to load a few extra neighbouring
  // blocks.

  // Synchronize with GL so that we can use the shared buffers.
  if (glSharing)
    glSharing(); // Could be replaced by a fence.

  // Enque the montage computation, and store the the result in the output
  // buffer.
  for (unsigned int i = 0; i < iters; ++i) {
    if (glSharing) {
      err = clEnqueueAcquireGLObjects(commandQueues[i], 1, &outBuffers[i], 0,
                                      nullptr, nullptr);
      checkClErrorCode(err, "clEnqueueAcquireGLObjects()");
    }

    cl_mem buffer = filterBuffers[i];
    int offset = nDiscard;
    if (allpass()) {
      buffer = rawBuffers[i];
      offset -= nDelay;
    }

    montageProcessor->process(montage.begin(), montage.end(), buffer,
                              outBuffers[i], xyzBuffer, commandQueues[i],
                              nMontage, offset);
    printBuffer("after_montage.txt", outBuffers[i], commandQueues[i]);
  }

  // Release the locked buffers and wait for all operations to finish.
  for (unsigned int i = 0; i < iters; ++i) {
    if (glSharing) {
      err = clEnqueueReleaseGLObjects(commandQueues[i], 1, &outBuffers[i], 0,
                                      nullptr, nullptr);
      checkClErrorCode(err, "clEnqueueReleaseGLObjects()");
    }

    err = clFinish(commandQueues[i]);
    checkClErrorCode(err, "clFinish()");
  }
}

void SignalProcessor::updateXyzBuffer(cl_command_queue queue, cl_mem xyzBuffer,
                                      const AbstractTrackTable *trackTable) {
  const int tracks = trackTable->rowCount();
  vector<float> xyz(3 * tracks);

  for (int i = 0; i < tracks; ++i) {
    Track t = trackTable->row(i);

    xyz[3 * i] = t.x;
    xyz[3 * i + 1] = t.y;
    xyz[3 * i + 2] = t.z;
  }

  cl_int err;
  size_t size = 3 * tracks * sizeof(float);
  err = clEnqueueWriteBuffer(queue, xyzBuffer, CL_FALSE, 0, size, xyz.data(), 0,
                             nullptr, nullptr);
  checkClErrorCode(err, "clEnqueueWriteBuffer()");

  err = clFinish(queue);
  checkClErrorCode(err, "clFinish()");
}

vector<string> SignalProcessor::collectLabels(AbstractTrackTable *trackTable) {
  vector<string> labels;
  labels.reserve(trackTable->rowCount());

  for (int i = 0; i < trackTable->rowCount(); ++i) {
    labels.push_back(trackTable->row(i).label);
  }

  return labels;
}

void SignalProcessor::updateMontage() {
  assert(ready());

  montageProcessor = make_unique<AlenkaSignal::MontageProcessor<float>>(
      nBlock + 2, fileChannels, montageCopyCount);

  clearMontage();

  vector<pair<string, cl_int>> montageCode;
  for (int i = 0; i < getTrackTable(file)->rowCount(); ++i) {
    const Track t = getTrackTable(file)->row(i);
    if (!t.hidden)
      montageCode.emplace_back(t.code, i);
  }

  if (maxMontageTracks < static_cast<int>(montageCode.size()))
    throwDetailed(runtime_error("Maximum montage size of " +
                                to_string(maxMontageTracks) + " exceeded"));

  auto montageTable = file->file->getDataModel()->montageTable();
  assert(0 < montageTable->rowCount());
  auto defaultTrackTable = montageTable->trackTable(0);

  updateXyzBuffer(commandQueues[0], xyzBuffer, defaultTrackTable);

  const string header =
      OpenDataFile::infoTable.getGlobalMontageHeader().toStdString();
  montage = makeMontage<float>(montageCode, context, header,
                               collectLabels(defaultTrackTable));
}

bool SignalProcessor::allpass() {
  return OpenDataFile::infoTable.getFrequencyMultipliersOn() == false &&
         filter->isAllpass();
}

void SignalProcessor::createXyzBuffer() {
  cl_int err;

  if (xyzBuffer) {
    err = clReleaseMemObject(xyzBuffer);
    checkClErrorCode(err, "clReleaseMemObject()");
  }

  cl_mem_flags flags = CL_MEM_READ_WRITE;
  size_t size = 3 * fileChannels * sizeof(float);

  xyzBuffer =
      clCreateBuffer(context->getCLContext(), flags, size, nullptr, &err);
  checkClErrorCode(err, "clCreateBuffer");
}
