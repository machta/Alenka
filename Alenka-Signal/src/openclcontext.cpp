#include "../include/AlenkaSignal/openclcontext.h"

#include "../include/AlenkaSignal/montage.h"

#include <clFFT.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>

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

} // namespace

namespace AlenkaSignal {

OpenCLContext::OpenCLContext(unsigned int platform, unsigned int device,
                             vector<cl_context_properties> properties) {
  cl_int err;

  // Retrieve the platform and device ids.
  cl_uint pCount = platform + 1;
  vector<cl_platform_id> platforms(pCount);

  err = clGetPlatformIDs(pCount, platforms.data(), &pCount);
  checkClErrorCode(err, "clGetPlatformIDs()");

  if (platform >= pCount)
    throw runtime_error("Platform ID " + to_string(platform) + " too high.");

  platformId = platforms[platform];

  cl_uint dCount = device + 1;
  vector<cl_device_id> devices(dCount);

  err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, dCount, devices.data(),
                       &dCount);
  checkClErrorCode(err, "clGetDeviceIDs()");

  if (device >= dCount)
    throw runtime_error("Device ID " + to_string(device) + " too high.");

  deviceId = devices[device];

  // Create the context.
  properties.push_back(CL_CONTEXT_PLATFORM);
  properties.push_back(reinterpret_cast<cl_context_properties>(platformId));
  properties.push_back(0);

  context =
      clCreateContext(properties.data(), 1, &deviceId, nullptr, nullptr, &err);
  checkClErrorCode(err, "clCreateContext()");

  // logToFile("OpenCLContext " << this << " created.");
}

OpenCLContext::~OpenCLContext() {
  cl_int err = clReleaseContext(context);
  checkClErrorCode(err, "clReleaseContext()");

  // logToFile("OpenCLContext " << this << " destroyed.");
}

string OpenCLContext::getPlatformInfo() const {
  cl_int err;
  cl_uint platformCount;

  err = clGetPlatformIDs(0, nullptr, &platformCount);
  checkClErrorCode(err, "clGetPlatformIDs()");

  vector<cl_platform_id> platformIDs(platformCount);
  err = clGetPlatformIDs(platformCount, platformIDs.data(), nullptr);
  checkClErrorCode(err, "clGetPlatformIDs()");

  // Build the string.
  string str;

  str += "Available platforms (" + to_string(platformCount) + "):";
  for (cl_uint i = 0; i < platformCount; ++i) {
    str += platformIDs[i] == getCLPlatform() ? "\n * " : "\n   ";
    str += platformInfo(platformIDs[i], CL_PLATFORM_NAME);
  }

  str += "\n\nSelected platform: ";
  str += platformInfo(getCLPlatform(), CL_PLATFORM_NAME);

  str += "\nVersion: ";
  str += platformInfo(getCLPlatform(), CL_PLATFORM_VERSION);

  str += "\nVendor: ";
  str += platformInfo(getCLPlatform(), CL_PLATFORM_VENDOR);

  str += "\nExtensions: ";
  str += platformInfo(getCLPlatform(), CL_PLATFORM_EXTENSIONS);

  return str;
}

string OpenCLContext::getDeviceInfo() const {
  cl_int err;

  cl_uint deviceCount;

  err = clGetDeviceIDs(getCLPlatform(), CL_DEVICE_TYPE_ALL, 0, nullptr,
                       &deviceCount);
  checkClErrorCode(err, "clGetDeviceIDs()");

  vector<cl_device_id> deviceIDs(deviceCount);
  err = clGetDeviceIDs(getCLPlatform(), CL_DEVICE_TYPE_ALL, deviceCount,
                       deviceIDs.data(), nullptr);
  checkClErrorCode(err, "clGetDeviceIDs()");

  // Build the string.
  string str;

  str += "Available devices (" + to_string(deviceCount) + "):";
  for (cl_uint i = 0; i < deviceCount; ++i) {
    str += deviceIDs[i] == getCLDevice() ? "\n * " : "\n   ";
    str += deviceInfo(deviceIDs[i], CL_DEVICE_NAME);
  }

  str += "\n\nSelected device: ";
  str += deviceInfo(getCLDevice(), CL_DEVICE_NAME);

  str += "\nVersion: ";
  str += deviceInfo(getCLDevice(), CL_DEVICE_VERSION);

  str += "\nVendor: ";
  str += deviceInfo(getCLDevice(), CL_DEVICE_VENDOR);

  str += "\nExtensions: ";
  checkClErrorCode(err, "clGetDeviceInfo()");
  str += deviceInfo(getCLDevice(), CL_DEVICE_EXTENSIONS);

  str += "\nDevice global memory size: ";
  cl_ulong memSize;
  err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_GLOBAL_MEM_SIZE,
                        sizeof(cl_ulong), &memSize, nullptr);
  checkClErrorCode(err, "clGetDeviceInfo()");
  stringstream ss;
  double memGigs = memSize / (1000. * 1000 * 1000);
  ss << memGigs;
  str += ss.str();
  str += " GB (this is the GPU memory size iff the sected device is a GPU)";

  return str;
}

void OpenCLContext::CCEC(cl_int val, string message, const char *file,
                         int line) {
  std::stringstream ss;

  ss << "Unexpected error code: ";
  ss << clErrorCodeToString(val);
  ss << ", required CL_SUCCESS. ";

  ss << message << " " << file << ":" << line;

  throw std::runtime_error(ss.str());
}

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

} // namespace AlenkaSignal
