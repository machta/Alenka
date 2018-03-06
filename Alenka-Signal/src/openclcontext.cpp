#include "../include/AlenkaSignal/openclcontext.h"

#include "../include/AlenkaSignal/montage.h"

#include <clFFT.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>

#include <detailedexception.h>

using namespace std;

namespace {

template <typename T> string errorCodeToString(T val) {
  using namespace std;

  stringstream ss;

  ss << dec << val << "(0x" << hex << val << dec << ")";

  return ss.str();
}

size_t platformInfoSize(cl_platform_id id, cl_platform_info name) {
  size_t size;

  cl_int err = clGetPlatformInfo(id, name, 0, nullptr, &size);
  checkClErrorCode(err, "clGetPlatformInfo()");

  return size;
}

string platformInfo(cl_platform_id id, cl_platform_info name) {
  size_t size = platformInfoSize(id, name);
  vector<char> tmp(size);

  cl_int err = clGetPlatformInfo(id, name, size, tmp.data(), nullptr);
  checkClErrorCode(err, "clGetPlatformInfo()");

  return string(tmp.begin(), --tmp.end());
}

size_t deviceInfoSize(cl_device_id id, cl_device_info name) {
  size_t size;

  cl_int err = clGetDeviceInfo(id, name, 0, nullptr, &size);
  checkClErrorCode(err, "clGetDeviceInfo()");

  return size;
}

string deviceInfo(cl_device_id id, cl_device_info name) {
  size_t size = deviceInfoSize(id, name);
  vector<char> tmp(size);

  cl_int err = clGetDeviceInfo(id, name, size, tmp.data(), nullptr);
  checkClErrorCode(err, "clGetDeviceInfo()");

  return string(tmp.begin(), --tmp.end());
}

vector<cl_platform_id> getPlatformIDs() {
  cl_int err;
  cl_uint platformCount;

  err = clGetPlatformIDs(0, nullptr, &platformCount);
  checkClErrorCode(err, "clGetPlatformIDs()");

  vector<cl_platform_id> platformIDs(platformCount);
  err = clGetPlatformIDs(platformCount, platformIDs.data(), nullptr);
  checkClErrorCode(err, "clGetPlatformIDs()");

  return platformIDs;
}

vector<cl_device_id> getDeviceIDs(const cl_platform_id platformID) {
  cl_int err;
  cl_uint deviceCount;

  err =
      clGetDeviceIDs(platformID, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
  checkClErrorCode(err, "clGetDeviceIDs()");

  vector<cl_device_id> deviceIDs(deviceCount);
  err = clGetDeviceIDs(platformID, CL_DEVICE_TYPE_ALL, deviceCount,
                       deviceIDs.data(), nullptr);
  checkClErrorCode(err, "clGetDeviceIDs()");

  return deviceIDs;
}

} // namespace

namespace AlenkaSignal {

OpenCLContext::OpenCLContext(const unsigned int platformIndex,
                             const unsigned int deviceIndex,
                             const vector<cl_context_properties> &properties) {
  cl_int err;

  const vector<cl_platform_id> platforms = getPlatformIDs();

  if (platformIndex >= platforms.size())
    throwDetailed(runtime_error("Platform ID " + to_string(platformIndex) +
                                " too high."));

  platformId = platforms[platformIndex];

  cl_uint dCount = deviceIndex + 1;
  const vector<cl_device_id> devices = getDeviceIDs(platformId);

  if (deviceIndex >= dCount)
    throwDetailed(
        runtime_error("Device ID " + to_string(deviceIndex) + " too high."));

  deviceId = devices[deviceIndex];

  // Create the context.
  vector<cl_context_properties> extraProperties{
      CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platformId),
      0};
  extraProperties.insert(extraProperties.begin(), properties.begin(),
                         properties.end());
  context = clCreateContext(extraProperties.data(), 1, &deviceId, nullptr,
                            nullptr, &err);
  checkClErrorCode(err, "clCreateContext()");

  // logToFile("OpenCLContext " << this << " created.");
}

OpenCLContext::~OpenCLContext() {
  cl_int err = clReleaseContext(context);
  checkClErrorCode(err, "clReleaseContext()");

  // logToFile("OpenCLContext " << this << " destroyed.");
}

string OpenCLContext::getPlatformInfo(const unsigned int platformIndex) {
  const vector<cl_platform_id> platformIDs = getPlatformIDs();

  // Build the string.
  string str;

  str += "Available platforms (" + to_string(platformIDs.size()) + "):";
  for (cl_uint i = 0; i < platformIDs.size(); ++i) {
    str += i == platformIndex ? "\n * " : "\n   ";
    str += platformInfo(platformIDs[i], CL_PLATFORM_NAME);
  }

  if (platformIndex < platformIDs.size()) {
    const cl_platform_id id = platformIDs[platformIndex];

    str += "\n\nSelected platform: ";
    str += platformInfo(id, CL_PLATFORM_NAME);

    str += "\nVersion: ";
    str += platformInfo(id, CL_PLATFORM_VERSION);

    str += "\nVendor: ";
    str += platformInfo(id, CL_PLATFORM_VENDOR);

    str += "\nExtensions: ";
    str += platformInfo(id, CL_PLATFORM_EXTENSIONS);
  }
  return str;
}

string OpenCLContext::getDeviceInfo(const unsigned int platformIndex,
                                    const unsigned int deviceIndex) {
  cl_int err;
  const vector<cl_platform_id> platformIDs = getPlatformIDs();

  if (platformIndex >= platformIDs.size())
    throwDetailed(runtime_error("Platform ID " + to_string(platformIndex) +
                                " too high."));

  const vector<cl_device_id> deviceIDs =
      getDeviceIDs(platformIDs[platformIndex]);

  // Build the string.
  string str;

  str += "Available devices (" + to_string(deviceIDs.size()) + "):";
  for (cl_uint i = 0; i < deviceIDs.size(); ++i) {
    str += i == deviceIndex ? "\n * " : "\n   ";
    str += deviceInfo(deviceIDs[i], CL_DEVICE_NAME);
  }

  if (deviceIndex < deviceIDs.size()) {
    const cl_device_id id = deviceIDs[deviceIndex];

    str += "\n\nSelected device: ";
    str += deviceInfo(id, CL_DEVICE_NAME);

    str += "\nVersion: ";
    str += deviceInfo(id, CL_DEVICE_VERSION);

    str += "\nVendor: ";
    str += deviceInfo(id, CL_DEVICE_VENDOR);

    str += "\nExtensions: ";
    str += deviceInfo(id, CL_DEVICE_EXTENSIONS);

    str += "\nDevice global memory size: ";
    cl_ulong memSize;
    err = clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong),
                          &memSize, nullptr);
    checkClErrorCode(err, "clGetDeviceInfo()");

    const double memGigs = memSize / (1000. * 1000 * 1000);
    str += to_string(memGigs);
    str += " GB (this is the GPU memory size iff the sected device is a GPU)";
  }

  return str;
}

// TODO: Test this with a unit test.
void OpenCLContext::CCEC(cl_int val, string message, const char *file,
                         int line) {
  std::stringstream ss;

  ss << "Unexpected error code: ";
  ss << clErrorCodeToString(val);
  ss << ", required CL_SUCCESS. ";

  ss << message << " " << file << ":" << line;

  throwDetailed(std::runtime_error(ss.str()));
}

// TODO: Test this with a unit test.
string OpenCLContext::clErrorCodeToString(cl_int code) {
#define CASE(a_)                                                               \
  case a_:                                                                     \
    return #a_

  switch (code) {
    CASE(CL_SUCCESS);
    CASE(CL_DEVICE_NOT_FOUND);
    CASE(CL_DEVICE_NOT_AVAILABLE);
    CASE(CL_COMPILER_NOT_AVAILABLE);
    CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
    CASE(CL_OUT_OF_RESOURCES);
    CASE(CL_OUT_OF_HOST_MEMORY);
    CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
    CASE(CL_MEM_COPY_OVERLAP);
    CASE(CL_IMAGE_FORMAT_MISMATCH);
    CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
    CASE(CL_BUILD_PROGRAM_FAILURE);
    CASE(CL_MAP_FAILURE);
    CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET);
    CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    // OpenCL 1.2
    CASE(CL_COMPILE_PROGRAM_FAILURE);
    CASE(CL_LINKER_NOT_AVAILABLE);
    CASE(CL_LINK_PROGRAM_FAILURE);
    CASE(CL_DEVICE_PARTITION_FAILED);
    CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);

    CASE(CL_INVALID_VALUE);
    CASE(CL_INVALID_DEVICE_TYPE);
    CASE(CL_INVALID_PLATFORM);
    CASE(CL_INVALID_DEVICE);
    CASE(CL_INVALID_CONTEXT);
    CASE(CL_INVALID_QUEUE_PROPERTIES);
    CASE(CL_INVALID_COMMAND_QUEUE);
    CASE(CL_INVALID_HOST_PTR);
    CASE(CL_INVALID_MEM_OBJECT);
    CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    CASE(CL_INVALID_IMAGE_SIZE);
    CASE(CL_INVALID_SAMPLER);
    CASE(CL_INVALID_BINARY);
    CASE(CL_INVALID_BUILD_OPTIONS);
    CASE(CL_INVALID_PROGRAM);
    CASE(CL_INVALID_PROGRAM_EXECUTABLE);
    CASE(CL_INVALID_KERNEL_NAME);
    CASE(CL_INVALID_KERNEL_DEFINITION);
    CASE(CL_INVALID_KERNEL);
    CASE(CL_INVALID_ARG_INDEX);
    CASE(CL_INVALID_ARG_VALUE);
    CASE(CL_INVALID_ARG_SIZE);
    CASE(CL_INVALID_KERNEL_ARGS);
    CASE(CL_INVALID_WORK_DIMENSION);
    CASE(CL_INVALID_WORK_GROUP_SIZE);
    CASE(CL_INVALID_WORK_ITEM_SIZE);
    CASE(CL_INVALID_GLOBAL_OFFSET);
    CASE(CL_INVALID_EVENT_WAIT_LIST);
    CASE(CL_INVALID_EVENT);
    CASE(CL_INVALID_OPERATION);
    CASE(CL_INVALID_GL_OBJECT);
    CASE(CL_INVALID_BUFFER_SIZE);
    CASE(CL_INVALID_MIP_LEVEL);
    CASE(CL_INVALID_GLOBAL_WORK_SIZE);
    CASE(CL_INVALID_PROPERTY);
    // OpenCL 1.2
    CASE(CL_INVALID_IMAGE_DESCRIPTOR);
    CASE(CL_INVALID_COMPILER_OPTIONS);
    CASE(CL_INVALID_LINKER_OPTIONS);
    CASE(CL_INVALID_DEVICE_PARTITION_COUNT);
  }

#undef CASE

  return "unknown code " + errorCodeToString(code);
}

void OpenCLContext::clfftInit() {
  clfftStatus errFFT;
  clfftSetupData setupData;

  errFFT = clfftInitSetupData(&setupData);
  assert(errFFT == CLFFT_SUCCESS);
  (void)errFFT;

  errFFT = clfftSetup(&setupData);
  assert(errFFT == CLFFT_SUCCESS);
  (void)errFFT;
}

void OpenCLContext::clfftDeinit() {
  clfftStatus errFFT = clfftTeardown();
  assert(errFFT == CLFFT_SUCCESS);
  (void)errFFT;
}

// This is for debugging purposes only, so it doesn't need to be tested.
// LCOV_EXCL_START
void OpenCLContext::printBuffer(FILE *file, float *data, int n) {
#ifndef NDEBUG
  for (int i = 0; i < n; ++i) {
    float tmp = data[i];
    if (std::isnan(tmp) /* || tmp < -1000*1000*1000 || tmp > 1000*1000*1000*/)
      tmp = 111111111;
    fprintf(file, "%f\n", tmp);
  }
#else
  (void)file;
  (void)data;
  (void)n;
#endif
}

void OpenCLContext::printBuffer(FILE *file, cl_mem buffer,
                                cl_command_queue queue) {
#ifndef NDEBUG
  cl_int err;

  size_t size;
  err = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size_t), &size, nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  vector<float> tmp(size / sizeof(float));

  err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, tmp.data(), 0,
                            nullptr, nullptr);
  checkClErrorCode(err, "clEnqueueReadBuffer");

  printBuffer(file, tmp.data(), size / sizeof(float));
#else
  (void)file;
  (void)buffer;
  (void)queue;
#endif
}

void OpenCLContext::printBuffer(const string &filePath, float *data, int n) {
#ifndef NDEBUG
  FILE *file = fopen(filePath.c_str(), "w");
  printBuffer(file, data, n);
  fclose(file);
#else
  (void)filePath;
  (void)data;
  (void)n;
#endif
}

void OpenCLContext::printBuffer(const string &filePath, cl_mem buffer,
                                cl_command_queue queue) {
#ifndef NDEBUG
  FILE *file = fopen(filePath.c_str(), "w");
  printBuffer(file, buffer, queue);
  fclose(file);
#else
  (void)filePath;
  (void)buffer;
  (void)queue;
#endif
}

void OpenCLContext::printBufferDouble(FILE *file, double *data, int n) {
#ifndef NDEBUG
  for (int i = 0; i < n; ++i) {
    double tmp = data[i];
    if (std::isnan(tmp) /* || tmp < -1000*1000*1000 || tmp > 1000*1000*1000*/)
      tmp = 111111111;
    fprintf(file, "%f\n", tmp);
  }
#else
  (void)file;
  (void)data;
  (void)n;
#endif
}

void OpenCLContext::printBufferDouble(FILE *file, cl_mem buffer,
                                      cl_command_queue queue) {
#ifndef NDEBUG
  cl_int err;

  size_t size;
  err = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size_t), &size, nullptr);
  checkClErrorCode(err, "clGetMemObjectInfo");

  vector<double> tmp(size / sizeof(double));

  err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, tmp.data(), 0,
                            nullptr, nullptr);
  checkClErrorCode(err, "clEnqueueReadBuffer");

  printBufferDouble(file, tmp.data(), size / sizeof(double));
#else
  (void)file;
  (void)buffer;
  (void)queue;
#endif
}

void OpenCLContext::printBufferDouble(const string &filePath, double *data,
                                      int n) {
#ifndef NDEBUG
  FILE *file = fopen(filePath.c_str(), "w");
  printBufferDouble(file, data, n);
  fclose(file);
#else
  (void)filePath;
  (void)data;
  (void)n;
#endif
}

void OpenCLContext::printBufferDouble(const string &filePath, cl_mem buffer,
                                      cl_command_queue queue) {
#ifndef NDEBUG
  FILE *file = fopen(filePath.c_str(), "w");
  printBufferDouble(file, buffer, queue);
  fclose(file);
#else
  (void)filePath;
  (void)buffer;
  (void)queue;
#endif
}
// LCOV_EXCL_STOP

} // namespace AlenkaSignal
