#include "openclcontext.h"

#include "error.h"

#include <algorithm>

using namespace std;

OpenCLContext::OpenCLContext(unsigned int platform, unsigned int device, cl_device_type deviceType)
{
	cl_uint pCount = platform + 1;
	cl_platform_id* platforms = new cl_platform_id[pCount];

	clGetPlatformIDs(pCount, platforms, &pCount);
	if (platform >= pCount)
	{
		throw runtime_error("Platform ID too high.");
	}

	pid = platforms[platform];

	cl_uint dCount = device + 1;
	cl_device_id* devices = new cl_device_id[dCount];

	clGetDeviceIDs(pid, deviceType, dCount, devices, &dCount);
	if (device >= dCount)
	{
		throw runtime_error("Device ID too high.");
	}

	cl_int err;
	context = clCreateContext(nullptr, 1, devices + device, nullptr, nullptr, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateContext()");

	did = devices[device];

    delete[] platforms;
    delete[] devices;
}

OpenCLContext::~OpenCLContext()
{
	clReleaseContext(context);
}

