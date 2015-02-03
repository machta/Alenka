#ifndef OPENCLPROGRAM_H
#define OPENCLPROGRAM_H

#include "openclcontext.h"

#include <CL/cl.h>

#include <sstream>

class OpenCLProgram
{
public:
	OpenCLProgram(const char* source, OpenCLContext context);
	~OpenCLProgram();

	cl_kernel createKernel(const char* kernelName)
	{
		cl_int err;
		cl_kernel kernel = clCreateKernel(program, kernelName, &err);
		if (err != CL_SUCCESS)
		{
			std::stringstream ss;
			ss << "clCreateKernel returned with error code " << err << ".";
			throw std::runtime_error(ss.str());
		}

		return kernel;
	}

private:
	cl_program program;
};

#endif // OPENCLPROGRAM_H
