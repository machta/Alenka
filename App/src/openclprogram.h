#ifndef OPENCLPROGRAM_H
#define OPENCLPROGRAM_H

#include "openclcontext.h"
#include "error.h"

#include <CL/cl.h>

#include <string>
#include <stdexcept>

class OpenCLProgram
{
public:
	OpenCLProgram(const char* source, OpenCLContext* context);
	~OpenCLProgram();

	cl_kernel createKernel(const char* kernelName)
	{
		if (compilationSuccessful() == false)
		{
			throw std::runtime_error("Cannot create kernel object from an OpenCLProgram that failed to compile.");
		}

		cl_int err;

		cl_kernel kernel = clCreateKernel(program, kernelName, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateKernel()");

		return kernel;
	}
	bool compilationSuccessful()
	{
		return !invalid;
	}
	std::string getCompilationLog();

private:
	cl_program program;

	bool invalid;
	OpenCLContext* clContext;
};

#endif // OPENCLPROGRAM_H
