#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../DataModel/kernelcache.h"
#include "../DataModel/opendatafile.h"
#include "../error.h"
#include "lrucache.h"

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <QString>

#include <cassert>
#include <chrono>
#include <cinttypes>
#include <functional>
#include <memory>
#include <set>
#include <vector>

namespace AlenkaFile {
class AbstractTrackTable;
}

namespace AlenkaSignal {
class OpenCLContext;
template <class T> class FilterProcessor;
template <class T> class MontageProcessor;
template <class T> class Filter;
} // namespace AlenkaSignal

/**
 * @brief A class used for retrieving the processed signal blocks.
 *
 * Before an object of this class is constructed or used an OpenGL context must
 * be set current. This is an example how to make it current in a method that
 * doesn't guarantee this:
 * @code{.cpp}
makeCurrent();
signalProcessor->setUpdateMontageFlag();
doneCurrent();
 * @endcode
 *
 */
class SignalProcessor {
  int trackCount = 0;
  bool updateMontageFlag = false;
  int maxMontageTracks = 0;

  int nBlock, nMontage, nSamples, M, nDelay, nDiscard;
  unsigned int parallelQueues, montageCopyCount, fileChannels;

  std::vector<cl_command_queue> commandQueues;
  std::vector<cl_mem> rawBuffers;
  std::vector<cl_mem> filterBuffers;
  cl_mem xyzBuffer = nullptr;
  QMetaObject::Connection xyzBufferConnection;
  std::unique_ptr<LRUCache<int, float>> cache;

  std::function<void()> glSharing;
  OpenDataFile *file;
  AlenkaSignal::OpenCLContext *context;
  std::vector<std::unique_ptr<AlenkaSignal::FilterProcessor<float>>>
      filterProcessors;
  std::unique_ptr<AlenkaSignal::MontageProcessor<float>> montageProcessor;
  std::vector<std::unique_ptr<AlenkaSignal::Montage<float>>> montage;
  int extraSamplesFront, extraSamplesBack;
  std::unique_ptr<AlenkaSignal::Filter<float>> filter;

public:
  SignalProcessor(unsigned int nBlock, unsigned int parallelQueues,
                  int montageCopyCount, std::function<void()> glSharing,
                  OpenDataFile *file, AlenkaSignal::OpenCLContext *context,
                  int extraSamplesFront = 0, int extraSamplesBack = 0);
  ~SignalProcessor();

  /**
   * @brief Returns the number of channels of the result signal block.
   */
  int getTrackCount() const { return trackCount; }

  int montageLength() { return nMontage; }

  /**
   * @brief Updates the currently used filter according to the values set in the
   * InfoTable.
   *
   * The filter is updated immediately.
   */
  void updateFilter();

  /**
   * @brief The montage will be updated next time it is needed.
   *
   * The update is lazy -- consecutive calls will do nothing. The reason for
   * this is that needles repeated compilation of the montage code used to cause
   * significant slowdown.
   */
  void setUpdateMontageFlag();

  /**
   * @brief Returns any block from indexSet ready to be used for rendering.
   * @param indexSet Requested block indexes.
   * @return An object wrapping the block index (and other info) and the vertex
   * arrays for accessing the data.
   *
   * In this method the synchronization between OpenGL and OpenCL is performed
   * through glFinish() and clFinish().
   *
   * Montage is updated if needed.
   *
   * When called ready() should be true.
   */
  void process(const std::vector<int> &indexVector,
               const std::vector<cl_mem> &outBuffers);

  /**
   * @brief Returns true if this object is ready for full operation.
   */
  bool ready() const { return file && 0 < trackCount; }

  template <class T>
  static std::string simplifyMontage(const std::string &str) {
    QString qstr = AlenkaSignal::Montage<T>::stripComments(str).c_str();
    return qstr.simplified().toStdString();
  }

  // TODO: Make sure you don't cache copy-montages.
  template <class T>
  static auto
  makeMontage(const std::vector<std::pair<std::string, cl_int>> &montageCode,
              AlenkaSignal::OpenCLContext *context, const std::string &header,
              const std::vector<std::string> &labels) {
    using namespace std;
#ifndef NDEBUG
    // TODO: Remove this after the compilation time issue is solved, or perhaps
    // log this info to a file.
    using namespace chrono;
    auto start = high_resolution_clock::now();
    int needToCompile = 0;
#endif
    std::vector<std::unique_ptr<AlenkaSignal::Montage<T>>> montage;

    for (const auto &e : montageCode) {
      auto sourceMontage = make_unique<AlenkaSignal::Montage<T>>(
          simplifyMontage<T>(e.first), context, header, labels);
      sourceMontage->setMontageIndex(e.second);

      if (AlenkaSignal::NormalMontage != sourceMontage->getMontageType()) {
        montage.push_back(std::move(sourceMontage));
      } else {
        const QString code = QString::fromStdString(sourceMontage->getSource());
        auto programPointer = OpenDataFile::kernelCache->find(code);

        if (!programPointer) {
          programPointer = sourceMontage->releaseProgram();
          assert(programPointer);
          OpenDataFile::kernelCache->insert(code, programPointer);
#ifndef NDEBUG
          ++needToCompile;
#endif
        }

        auto m = AlenkaSignal::Montage<T>::fromProgram(programPointer);
        montage.emplace_back(m);
      }
    }

#ifndef NDEBUG
    auto end = high_resolution_clock::now();
    nanoseconds time = end - start;
    string str = "Need to compile " + to_string(needToCompile) + " montages: " +
                 to_string(static_cast<double>(time.count()) / 1000 / 1000) +
                 " ms";
    if (needToCompile > 0) {
      logToFileAndConsole(str);
    } else {
      logToFile(str);
    }
#endif
    return montage;
  }

  static std::pair<std::int64_t, std::int64_t>
  blockIndexToSampleRange(int index, unsigned int blockSize) {
    using namespace std;

    int64_t from = index * (blockSize - 1);
    int64_t to = from + blockSize - 1;

    return make_pair(from, to);
  }

  static void updateXyzBuffer(cl_command_queue queue, cl_mem xyzBuffer,
                              const AlenkaFile::AbstractTrackTable *trackTable);

  static std::vector<std::string>
  collectLabels(AlenkaFile::AbstractTrackTable *trackTable);

private:
  /**
   * @brief This method actually (unlike setUpdateMontageFlag()) updates the
   * MontageProcessor object.
   */
  void updateMontage();
  void clearMontage() { montage.clear(); }
  bool allpass();
  void createXyzBuffer();
};

#endif // SIGNALPROCESSOR_H
