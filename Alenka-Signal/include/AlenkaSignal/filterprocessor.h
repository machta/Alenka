#ifndef ALENKASIGNAL_FILTERPROCESSOR_H
#define ALENKASIGNAL_FILTERPROCESSOR_H

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <cassert>
#include <vector>

typedef size_t clfftPlanHandle;

namespace AlenkaSignal {

class OpenCLContext;

enum class WindowFunction { None, Hamming, Blackman };

/**
 * @brief This class handles filtering of data blocks.
 */
template <class T> class FilterProcessor {
  unsigned int blockLength, blockChannels;
  int M;
  bool coefficientsChanged = false;
  std::vector<T> coefficients;

  cl_kernel filterKernel;
  cl_kernel zeroKernel;
  cl_mem filterBuffer;

  clfftPlanHandle fftPlan;
  clfftPlanHandle fftPlanBatch;
  clfftPlanHandle ifftPlanBatch;

public:
  FilterProcessor(unsigned int blockLength, unsigned int blockChannels,
                  OpenCLContext *context);
  ~FilterProcessor();

  void process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue);

  void changeFilter(const std::vector<T> &coefficients) {
    coefficientsChanged = true;
    M = static_cast<int>(coefficients.size());
    this->coefficients = coefficients;
  }
  void changeSampleFilter(int M, const std::vector<T> &samples);
  void applyWindow(WindowFunction windowFunction);

  int delaySamples() const { return (M - 1) / 2; }
  int discardSamples() const { return M - 1; }

  const std::vector<T> &getCoefficients() const { return coefficients; }
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_FILTERPROCESSOR_H
