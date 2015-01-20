#ifndef PROGRAM_H
#define PROGRAM_H

#include "context.h"

#include <CL/cl.h>

#include <sstream>

class Program
{
public:
	Program(const char* source, Context context);
	~Program();

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

#endif // PROGRAM_H
