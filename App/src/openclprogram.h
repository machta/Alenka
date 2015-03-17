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
	OpenCLProgram(FILE* source, OpenCLContext* context) : context(context)
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
	OpenCLProgram(std::string source, OpenCLContext* context) : context(context)
	{
		construct(source);
	}
	~OpenCLProgram();

	cl_kernel createKernel(const std::string& kernelName)
	{
		if (compilationSuccessful() == false)
		{
			throw std::runtime_error("Cannot create kernel object from an OpenCLProgram that failed to compile.");
		}

		cl_int err;

		cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateKernel()");

		return kernel;
	}
	bool compilationSuccessful() const
	{
		return !invalid;
	}
	std::string getCompilationLog() const;

private:
	cl_program program;

	bool invalid;
	OpenCLContext* context;

	void construct(const std::string& source);
};

#endif // OPENCLPROGRAM_H
