#include "openclcontext.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

#if defined WIN_BUILD
//#include <QtPlatformHeaders/QWGLNativeContext>
#include <windows.h>
#elif defined UNIX_BUILD
//#include <QtPlatformHeaders/QGLXNativeContext>
#include <GL/glx.h>
#endif

using namespace std;

OpenCLContext::OpenCLContext(unsigned int platform, unsigned int device, QOpenGLContext* parentContext)
{
	cl_int err;

	// Retrieve the platform and device ids.
	cl_uint pCount = platform + 1;
	cl_platform_id* platforms = new cl_platform_id[pCount];

	err = clGetPlatformIDs(pCount, platforms, &pCount);
	checkClErrorCode(err, "clGetPlatformIDs()");

	if (platform >= pCount)
	{
		stringstream ss;
		ss << "Platform ID " << platform << " too high.";
		throw runtime_error(ss.str());
	}

	platformId = platforms[platform];

	cl_uint dCount = device + 1;
	cl_device_id* devices = new cl_device_id[dCount];

	err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, dCount, devices, &dCount);
	checkClErrorCode(err, "clGetDeviceIDs()");

	if (device >= dCount)
	{
		stringstream ss;
		ss << "Device ID " << device << " too high.";
		throw runtime_error(ss.str());
	}

	deviceId = devices[device];

	// Create the context.
	vector<cl_context_properties> properties {CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platformId)};

	if (parentContext != nullptr)
	{
#if defined WIN_BUILD
		properties.push_back(CL_GL_CONTEXT_KHR);
		properties.push_back(reinterpret_cast<cl_context_properties>(wglGetCurrentContext()));

		properties.push_back(CL_WGL_HDC_KHR);
		properties.push_back(reinterpret_cast<cl_context_properties>(wglGetCurrentDC()));
#elif defined UNIX_BUILD
		properties.push_back(CL_GL_CONTEXT_KHR);
		properties.push_back(reinterpret_cast<cl_context_properties>(glXGetCurrentContext()));

		properties.push_back(CL_GLX_DISPLAY_KHR);
		properties.push_back(reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()));
#endif
	}

	properties.push_back(0);

	context = clCreateContext(properties.data(), 1, &deviceId, nullptr, nullptr, &err);
	checkClErrorCode(err, "clCreateContext()");

	delete[] platforms;
	delete[] devices;
}

OpenCLContext::~OpenCLContext()
{
	cl_int err = clReleaseContext(context);
	checkClErrorCode(err, "clReleaseContext()");
}

string OpenCLContext::getPlatformInfo() const
{
	cl_int err;	

	cl_uint platformCount;

	err = clGetPlatformIDs(0, nullptr, &platformCount);
	checkClErrorCode(err, "clGetPlatformIDs()");

	cl_platform_id* platformIDs = new cl_platform_id[platformCount];

	err = clGetPlatformIDs(platformCount, platformIDs, nullptr);
	checkClErrorCode(err, "clGetPlatformIDs()");

	// Find the maximum size needed for the values.
	size_t size, maxSize = 0;

	for (cl_uint i = 0; i < platformCount; ++i)
	{
		err = clGetPlatformInfo(platformIDs[i], CL_PLATFORM_VERSION, 0, nullptr, &size);
		checkClErrorCode(err, "clGetPlatformInfo()");
		maxSize = max(maxSize, size);

		err = clGetPlatformInfo(platformIDs[i], CL_PLATFORM_NAME, 0, nullptr, &size);
		checkClErrorCode(err, "clGetPlatformInfo()");
		maxSize = max(maxSize, size);

		err = clGetPlatformInfo(platformIDs[i], CL_PLATFORM_VENDOR, 0, nullptr, &size);
		checkClErrorCode(err, "clGetPlatformInfo()");
		maxSize = max(maxSize, size);

		err = clGetPlatformInfo(platformIDs[i], CL_PLATFORM_EXTENSIONS, 0, nullptr, &size);
		checkClErrorCode(err, "clGetPlatformInfo()");
		maxSize = max(maxSize, size);
	}

	// Build the string.
	char* tmp = new char[maxSize];
	string str;

	str += "Available platforms:";
	for (cl_uint i = 0; i < platformCount; ++i)
	{
		if (platformIDs[i] == getCLPlatform())
			str += "\n * ";
		else
			str += "\n   ";

		err = clGetPlatformInfo(platformIDs[i], CL_PLATFORM_NAME, maxSize, tmp, nullptr);
		checkClErrorCode(err, "clGetPlatformInfo()");
		str += tmp;
	}

	str += "\n\nSelected platform: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_NAME, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetPlatformInfo()");
	str += tmp;

	str += "\nVersion: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VERSION, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetPlatformInfo()");
	str += tmp;

	str += "\nVendor: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_VENDOR, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetPlatformInfo()");
	str += tmp;

	str += "\nExtensions: ";
	err = clGetPlatformInfo(getCLPlatform(), CL_PLATFORM_EXTENSIONS, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetPlatformInfo()");
	str += tmp;

	delete[] tmp;

	return str;
}

string OpenCLContext::getDeviceInfo() const
{
	cl_int err;

	cl_uint deviceCount;

	err = clGetDeviceIDs(getCLPlatform(), CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
	checkClErrorCode(err, "clGetDeviceIDs()");

	cl_device_id* deviceIDs = new cl_device_id[deviceCount];

	err = clGetDeviceIDs(getCLPlatform(), CL_DEVICE_TYPE_ALL, deviceCount, deviceIDs, nullptr);
	checkClErrorCode(err, "clGetDeviceIDs()");

	// Find the maximum size needed for the value.
	size_t size, maxSize = 0;

	for (cl_uint i = 0; i < deviceCount; ++i)
	{
		err = clGetDeviceInfo(deviceIDs[i], CL_DEVICE_VERSION, 0, nullptr, &size);
		checkClErrorCode(err, "clGetDeviceInfo()");
		maxSize = max(maxSize, size);

		err = clGetDeviceInfo(deviceIDs[i], CL_DEVICE_NAME, 0, nullptr, &size);
		checkClErrorCode(err, "clGetDeviceInfo()");
		maxSize = max(maxSize, size);

		err = clGetDeviceInfo(deviceIDs[i], CL_DEVICE_VENDOR, 0, nullptr, &size);
		checkClErrorCode(err, "clGetDeviceInfo()");
		maxSize = max(maxSize, size);

		err = clGetDeviceInfo(deviceIDs[i], CL_DEVICE_EXTENSIONS, 0, nullptr, &size);
		checkClErrorCode(err, "clGetDeviceInfo()");
		maxSize = max(maxSize, size);
	}

	// Build the string.
	char* tmp = new char[maxSize];
	string str;

	str += "Available devices:";
	for (cl_uint i = 0; i < deviceCount; ++i)
	{
		if (deviceIDs[i] == getCLDevice())
			str += "\n * ";
		else
			str += "\n   ";

		err = clGetDeviceInfo(deviceIDs[i], CL_DEVICE_NAME, maxSize, tmp, nullptr);
		checkClErrorCode(err, "clGetDeviceInfo()");
		str += tmp;
	}

	str += "\n\nSelected device: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_NAME, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetDeviceInfo()");
	str += tmp;

	str += "\nVersion: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VERSION, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetDeviceInfo()");
	str += tmp;

	str += "\nVendor: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_VENDOR, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetDeviceInfo()");
	str += tmp;

	str += "\nExtensions: ";
	err = clGetDeviceInfo(getCLDevice(), CL_DEVICE_EXTENSIONS, maxSize, tmp, nullptr);
	checkClErrorCode(err, "clGetDeviceInfo()");
	str += tmp;

	delete[] tmp;

	return str;
}

