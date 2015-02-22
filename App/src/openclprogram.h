#ifndef OPENCLPROGRAM_H
#define OPENCLPROGRAM_H

#include "openclcontext.h"
#include "error.h"

#include <CL/cl_gl.h>

#include <cstdio>
#include <string>
#include <stdexcept>

class OpenCLProgram
{
public:
	OpenCLProgram(FILE* source, OpenCLContext* context) : clContext(context)
	{
		fseek(source, 0, SEEK_END);
		size_t size = ftell(source);

		char* tmp = new char[size + 1];
		tmp[size] = 0;

		rewind(source);
		freadChecked(tmp, sizeof(char), size, source);

		construct(tmp);

		delete[] tmp;
	}
	OpenCLProgram(std::string source, OpenCLContext* context) : clContext(context)
	{
		construct(source);
	}
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

	void construct(const std::string& source);
};

#endif // OPENCLPROGRAM_H
