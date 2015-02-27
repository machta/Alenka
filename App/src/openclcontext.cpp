#include "openclcontext.h"

#include "error.h"

#if defined WIN_BUILD
#include <QtPlatformHeaders/QWGLNativeContext>
#elif defined UNIX_BUILD
#include <QtPlatformHeaders/QGLXNativeContext>
#endif

#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

OpenCLContext::OpenCLContext(unsigned int platform, unsigned int device, cl_device_type deviceType, QOpenGLContext* parentContext)
{
	// Retrieve the platform and device ids.
	cl_uint pCount = platform + 1;
	cl_platform_id* platforms = new cl_platform_id[pCount];

	clGetPlatformIDs(pCount, platforms, &pCount);
	if (platform >= pCount)
	{
		throw runtime_error("Platform ID too high.");
	}

	platformId = platforms[platform];

	cl_uint dCount = device + 1;
	cl_device_id* devices = new cl_device_id[dCount];

	clGetDeviceIDs(platformId, deviceType, dCount, devices, &dCount);
	if (device >= dCount)
	{
		throw runtime_error("Device ID too high.");
	}

	deviceId = devices[device];

	// Create the context.
	cl_int err;

	vector<cl_context_properties> properties {CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platformId)};

	if (parentContext != nullptr)
	{
		properties.push_back(CL_GL_CONTEXT_KHR);

		QVariant nativeHandle = parentContext->nativeHandle();
		checkErrorCode(nativeHandle.isNull(), false, "parentContext->nativeHandle()");

#if defined WIN_BUILD
		if (nativeHandle.canConvert<QWGLNativeContext>())
		{
			auto nativeContext = nativeHandle.value<QWGLNativeContext>();
			properties.push_back(reinterpret_cast<cl_context_properties>(nativeContext.context()));
		}
		else
		{
			throw runtime_error("Cannot convert to QWGLNativeContext.");
		}
#elif defined UNIX_BUILD
		if (nativeHandle.canConvert<QGLXNativeContext>())
		{
			auto nativeContext = nativeHandle.value<QGLXNativeContext>();
			properties.push_back(reinterpret_cast<cl_context_properties>(nativeContext.context()));
		}
		else
		{
			throw runtime_error("Cannot convert to QGLXNativeContext.");
		}
#endif
	}

	properties.push_back(0);

	context = clCreateContext(properties.data(), 1, &deviceId, nullptr, nullptr, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateContext()");

	delete[] platforms;
	delete[] devices;
}

OpenCLContext::~OpenCLContext()
{
	clReleaseContext(context);
}

string OpenCLContext::getPlatformInfo()
{
	cl_int err;
	size_t size, maxSize = 0;

	// Find the maximum size needed for the value.
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VERSION, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	maxSize = max(maxSize, size);

	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_NAME, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	maxSize = max(maxSize, size);

	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VENDOR, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	maxSize = max(maxSize, size);

	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_EXTENSIONS, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	maxSize = max(maxSize, size);

	// Build the string.
	char* tmp = new char[maxSize];
	string str;

	str += "Version: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VERSION, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	str += tmp;

	str += "\nName: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_NAME, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	str += tmp;

	str += "\nVendor: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VENDOR, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	str += tmp;

	str += "\nExtensions: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_EXTENSIONS, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetPlatformInfo()");
	str += tmp;

	delete[] tmp;

	return str;
}

string OpenCLContext::getDeviceInfo()
{
	cl_int err;
	size_t size, maxSize = 0;

	// Find the maximum size needed for the value.
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VERSION, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	maxSize = max(maxSize, size);

	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_NAME, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	maxSize = max(maxSize, size);

	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VENDOR, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	maxSize = max(maxSize, size);

	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_EXTENSIONS, 0, nullptr, &size);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	maxSize = max(maxSize, size);

	// Build the string.
	char* tmp = new char[maxSize];
	string str;

	str += "Version: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VERSION, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	str += tmp;

	str += "\nName: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_NAME, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	str += tmp;

	str += "\nVendor: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VENDOR, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	str += tmp;

	str += "\nExtensions: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_EXTENSIONS, maxSize, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetDeviceInfo()");
	str += tmp;

	delete[] tmp;

	return str;
}

