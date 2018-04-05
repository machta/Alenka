#include "../include/AlenkaSignal/openclprogram.h"

#include "../include/AlenkaSignal/openclcontext.h"

#include <cassert>
#include <cstdlib>
#include <memory>

using namespace std;

namespace AlenkaSignal {

OpenCLProgram::OpenCLProgram(const string &source, OpenCLContext *context)
    : context(context), source(source) {
  cl_int err;

  const char *sourcePointer = source.c_str();
  size_t size = source.size();

  program = clCreateProgramWithSource(context->getCLContext(), 1,
                                      &sourcePointer, &size, &err);
  checkClErrorCode(err, "clCreateProgramWithSource()");

  build();
}

OpenCLProgram::~OpenCLProgram() {
  cl_int err = clReleaseProgram(program);
  checkClErrorCode(err, "clReleaseProgram()");
}

cl_kernel OpenCLProgram::createKernel(const string &kernelName) const {
  if (CL_SUCCESS == compileStatus()) {
    cl_int err;
    cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &err);
    checkClErrorCode(err, "clCreateKernel()");

    return kernel;
  } else {
    return nullptr;
  }
}

string OpenCLProgram::getCompileLog() const {
  size_t logLength;

  cl_int err =
      clGetProgramBuildInfo(program, context->getCLDevice(),
                            CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLength);
  checkClErrorCode(err, "clGetProgramBuildInfo()");

  unique_ptr<char[]> tmp(new char[logLength + 1]);
  tmp[logLength] = 0;

  err = clGetProgramBuildInfo(program, context->getCLDevice(),
                              CL_PROGRAM_BUILD_LOG, logLength, tmp.get(),
                              nullptr);
  checkClErrorCode(err, "clGetProgramBuildInfo()");

  return string(tmp.get());
}

string OpenCLProgram::makeErrorMessage(const string &msg) const {
  string str = msg + " compilation failed with " +
               OpenCLContext::clErrorCodeToString(buildError) + ":\n" +
               getCompileLog();

  if (!source.empty())
    str += "Source:\n" + source;

  return str;
}

vector<unsigned char> *OpenCLProgram::getBinary() const {
  size_t size = sizeof(size_t);
  cl_int err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                                sizeof(size_t), &size, nullptr);
  checkClErrorCode(err, "clGetProgramInfo()");

  auto binary = new vector<unsigned char>();

  if (size > 0) {
    binary->resize(size);

    unsigned char *value = binary->data();

    err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, size, &value, nullptr);
    checkClErrorCode(err, "clGetProgramInfo()");
  }

  return binary;
}

OpenCLProgram::OpenCLProgram(const vector<unsigned char> *binary,
                             OpenCLContext *context)
    : context(context) {
  cl_int err, status;
  auto device = context->getCLDevice();
  size_t size = binary->size();
  const unsigned char *binaryPtr = binary->data();

  program = clCreateProgramWithBinary(context->getCLContext(), 1, &device,
                                      &size, &binaryPtr, &status, &err);
  checkClErrorCode(err, "clCreateProgramWithBinary()");

  build();
}

void OpenCLProgram::build() {
  buildError = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

  if (CL_SUCCESS != buildError) {
    cl_build_status status;

    cl_int err = clGetProgramBuildInfo(
        program, context->getCLDevice(), CL_PROGRAM_BUILD_STATUS,
        sizeof(cl_build_status), &status, nullptr);
    checkClErrorCode(err, "clGetProgramBuildInfo()");

    assert(status != CL_BUILD_IN_PROGRESS);
  }
}

} // namespace AlenkaSignal
