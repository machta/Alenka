#ifndef CONTEXT_H
#define CONTEXT_H

#include <CL/cl.h>

class Context
{
public:
	Context(unsigned int platform, unsigned int device, cl_device_type deviceType);
	~Context();

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

#endif // CONTEXT_H

