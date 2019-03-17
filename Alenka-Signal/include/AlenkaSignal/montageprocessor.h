#ifndef ALENKASIGNAL_MONTAGEPROCESSOR_H
#define ALENKASIGNAL_MONTAGEPROCESSOR_H

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <type_traits>
#include <vector>

#include "montage.h"

namespace AlenkaSignal {
/**
 * @brief This class handles computation of montages.
 */
template <class T> class MontageProcessor {
  cl_int inputRowLength, inputRowCount, outputCopyCount;

public:
  /**
   * @brief MontageProcessor constructor.
   */
  MontageProcessor(unsigned int inputRowLength, int inputRowCount,
                   int outputCopyCount = 1)
      : inputRowLength(inputRowLength), inputRowCount(inputRowCount),
        outputCopyCount(outputCopyCount) {}

  /**
   * @brief Enqueues all commands required for montage computation.
   *
   * A range of iterators or pointers is expected. If you dereference
   * montageBegin twice, you should get a Montage<T> object. So you can use a
   * container of raw pointers or unique pointers.
   */
  template <class Iter>
  void process(Iter montageBegin, Iter montageEnd, cl_mem inBuffer,
               cl_mem outBuffer, cl_mem xyzBuffer, cl_command_queue queue,
               cl_int outputRowLength, cl_int inputRowOffset = 0) {
    static_assert(
        std::is_same<
            typename std::remove_reference<decltype(**montageBegin)>::type,
            Montage<T>>::value,
        "An iterator/pointer to 'Montage<T> *' is expected");

    checkBufferSizes(inBuffer, outBuffer, xyzBuffer, outputRowLength,
                     std::distance(montageBegin, montageEnd));

    int i = 0;
    for (Iter it = montageBegin; it != montageEnd; ++it) {
      const auto &mont = *it;
      int copyIndex =
          CopyMontage == mont->getMontageType() ? mont->copyMontageIndex() : -1;
      processOneMontage(inBuffer, outBuffer, xyzBuffer, queue, outputRowLength,
                        inputRowOffset, i++, mont->getMontageIndex(),
                        mont->getKernel(), copyIndex);
    }
  }

private:
  void checkBufferSizes(cl_mem inBuffer, cl_mem outBuffer, cl_mem xyzBuffer,
                        cl_int outputRowLength, size_t montageSize);
  void processOneMontage(cl_mem inBuffer, cl_mem outBuffer, cl_mem xyzBuffer,
                         cl_command_queue queue, cl_int outputRowLength,
                         cl_int inputRowOffset, cl_int index,
                         cl_int montageIndex, cl_kernel kernel, int copyIndex);
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_MONTAGEPROCESSOR_H
