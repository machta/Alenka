#include "../include/AlenkaSignal/montageprocessor.h"

#include "../include/AlenkaSignal/openclcontext.h"

#include <detailedexception.h>

using namespace std;

namespace AlenkaSignal {

template <class T>
void MontageProcessor<T>::checkBufferSizes(cl_mem inBuffer, cl_mem outBuffer,
                                           cl_mem xyzBuffer,
                                           cl_int outputRowLength,
                                           size_t montageSize) {
  size_t inSize;
  cl_int err = clGetMemObjectInfo(inBuffer, CL_MEM_SIZE, sizeof(size_t),
                                  &inSize, nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  const size_t minInSize = inputRowLength * inputRowCount * sizeof(T);
  if (inSize < minInSize) {
    const string msg = "The input buffer is too small: expected at least " +
                       to_string(minInSize) + ", got " + to_string(inSize);
    throwDetailed(runtime_error(msg));
  }

  size_t outSize;
  err = clGetMemObjectInfo(outBuffer, CL_MEM_SIZE, sizeof(size_t), &outSize,
                           nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  const size_t minOutSize =
      outputRowLength * montageSize * outputCopyCount * sizeof(T);
  if (outSize < minOutSize) {
    const string msg = "The output buffer is too small: expected at least " +
                       to_string(minOutSize) + ", got " + to_string(outSize);
    throwDetailed(runtime_error(msg));
  }

  size_t xyzSize;
  err = clGetMemObjectInfo(xyzBuffer, CL_MEM_SIZE, sizeof(size_t), &xyzSize,
                           nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  const size_t minXyzSize = inputRowCount * 3 * sizeof(T);
  if (xyzSize < minXyzSize) {
    const string msg = "The xyz buffer is too small: expected at least " +
                       to_string(minXyzSize) + ", got " + to_string(xyzSize);
    throwDetailed(runtime_error(msg));
  }
}

template <class T>
void MontageProcessor<T>::processOneMontage(
    cl_mem inBuffer, cl_mem outBuffer, cl_mem xyzBuffer, cl_command_queue queue,
    cl_int outputRowLength, cl_int inputRowOffset, cl_int index,
    cl_int montageIndex, cl_kernel kernel, int copyIndex) {
  cl_int err;
  int pi = 0;

  err = clSetKernelArg(kernel, pi++, sizeof(cl_mem), &inBuffer);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_mem), &outBuffer);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &inputRowLength);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &inputRowOffset);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &inputRowCount);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &outputRowLength);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &index);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &montageIndex);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &outputCopyCount);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  err = clSetKernelArg(kernel, pi++, sizeof(cl_mem), &xyzBuffer);
  checkClErrorCode(err, "clSetKernelArg(" << pi << ")");

  if (0 <= copyIndex) {
    err = clSetKernelArg(kernel, pi++, sizeof(cl_int), &copyIndex);
    checkClErrorCode(err, "clSetKernelArg(" << pi << ")");
  }

  size_t globalWorkSize = outputRowLength;

  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalWorkSize,
                               nullptr, 0, nullptr, nullptr);
  checkClErrorCode(err, "clEnqueueNDRangeKernel()");
}

template class MontageProcessor<float>;
template class MontageProcessor<double>;

} // namespace AlenkaSignal
