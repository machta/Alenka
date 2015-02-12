#ifndef OPENCLPROGRAM_H
#define OPENCLPROGRAM_H

#include "openclcontext.h"
#include "error.h"

#include <CL/cl.h>

class OpenCLProgram
{
public:
	OpenCLProgram(const char* source, OpenCLContext context);
	~OpenCLProgram();

	cl_kernel createKernel(const char* kernelName)
	{
		cl_int err;

		cl_kernel kernel = clCreateKernel(program, kernelName, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateKernel()");

		return kernel;
	}

private:
	cl_program program;
};

#endif // OPENCLPROGRAM_H
