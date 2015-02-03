#ifndef OPENCLCONTEXT_H
#define OPENCLCONTEXT_H

#include <CL/cl.h>

class OpenCLContext
{
public:
	OpenCLContext(unsigned int platform, unsigned int device, cl_device_type deviceType);
	~OpenCLContext();

	cl_context getCLContext()
	{
		return context;
	}
	cl_platform_id getCLPlatform()
	{
		return pid;
	}
	cl_device_id getCLDevice()
	{
		return did;
	}

private:
	cl_context context;
	cl_platform_id pid;
	cl_device_id did;
};

#endif // OPENCLCONTEXT_H

