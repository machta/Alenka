#include "openclcontext.h"

#include "error.h"

#if defined WIN_BUILD
#include <QtPlatformHeaders/QWGLNativeContext>
#elif defined UNIX_BUILD
#include <QtPlatformHeaders/QGLXNativeContext>
#endif

#include <algorithm>
#include <vector>

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

