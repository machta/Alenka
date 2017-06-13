#include "../include/AlenkaSignal/montageprocessor.h"

#include "../include/AlenkaSignal/montage.h"
#include "../include/AlenkaSignal/openclcontext.h"

using namespace std;

namespace AlenkaSignal {

template <class T>
void MontageProcessor<T>::checkBufferSizes(cl_mem inBuffer, cl_mem outBuffer,
                                           cl_mem xyzBuffer,
                                           cl_int outputRowLength,
                                           size_t montageSize) {
  cl_int err;
  size_t inSize, outSize, xyzSize;

  err = clGetMemObjectInfo(inBuffer, CL_MEM_SIZE, sizeof(size_t), &inSize,
                           nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  if (inSize < inputRowLength * inputRowCount * sizeof(T))
    throw runtime_error("MontageProcessor: the inBuffer is too small.");

  err = clGetMemObjectInfo(outBuffer, CL_MEM_SIZE, sizeof(size_t), &outSize,
                           nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  if (outSize < outputRowLength * montageSize * outputCopyCount * sizeof(T))
    throw runtime_error("MontageProcessor: the outBuffer is too small.");

  err = clGetMemObjectInfo(xyzBuffer, CL_MEM_SIZE, sizeof(size_t), &xyzSize,
                           nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  if (xyzSize < inputRowCount * 3 * sizeof(T))
    throw runtime_error("MontageProcessor: the xyzBuffer is too small.");
}

template <class T>
void MontageProcessor<T>::processOneMontage(cl_mem inBuffer, cl_mem outBuffer,
                                            cl_mem xyzBuffer,
                                            cl_command_queue queue,
                                            cl_int outputRowLength,
                                            cl_int inputRowOffset, cl_int index,
                                            cl_kernel kernel, int copyIndex) {
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
