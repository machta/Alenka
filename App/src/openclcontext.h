#ifndef OPENCLCONTEXT_H
#define OPENCLCONTEXT_H

#include <QOpenGLContext>
#include <CL/cl_gl.h>

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

private:
	cl_context context;
	cl_platform_id platformId;
	cl_device_id deviceId;
};

#endif // OPENCLCONTEXT_H

