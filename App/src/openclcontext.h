#ifndef OPENCLCONTEXT_H
#define OPENCLCONTEXT_H

#include <QOpenGLContext>
#include <CL/cl_gl.h>

#include <string>

class OpenCLContext
{
public:
	OpenCLContext(unsigned int platform, unsigned int device, cl_device_type deviceType, QOpenGLContext* parentContext = nullptr);
	~OpenCLContext();

	cl_context getCLContext()
	{
		return context;
	}
	cl_platform_id getCLPlatform()
	{
		return platformId;
	}
	cl_device_id getCLDevice()
	{
		return deviceId;
	}
	std::string getPlatformInfo();
	std::string getDeviceInfo();
private:
	cl_context context;
	cl_platform_id platformId;
	cl_device_id deviceId;
};

#define SIGNAL_PROCESSOR_CONTEXT_PARAMETERS PROGRAM_OPTIONS["platform"].as<int>(), PROGRAM_OPTIONS["device"].as<int>(), CL_DEVICE_TYPE_ALL
#define FILTER_CONTEXT_PARAMETERS PROGRAM_OPTIONS["platform"].as<int>(), 0, CL_DEVICE_TYPE_CPU

#endif // OPENCLCONTEXT_H

