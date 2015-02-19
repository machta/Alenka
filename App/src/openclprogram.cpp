#include "openclprogram.h"

#include "error.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>

using namespace std;

OpenCLProgram::OpenCLProgram(const char* source, OpenCLContext* context) : clContext(context)
{
	cl_int err;

	FILE* file = fopen(source, "rb");
	checkNotErrorCode(file, nullptr, "File '" << source << "' could not be opened.");

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);

	char* tmp = new char[size + 1];
	tmp[size] = 0;

	rewind(file);
	freadChecked(tmp, sizeof(char), size, file);

	program = clCreateProgramWithSource(clContext->getCLContext(), 1, const_cast<const char**>(&tmp), &size, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateProgramWithSource()");

	delete[] tmp;

	err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

	if (err != CL_SUCCESS)
	{
		cl_build_status status;

		cl_int err2 = clGetProgramBuildInfo(program, clContext->getCLDevice(), CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, nullptr);
		checkErrorCode(err2, CL_SUCCESS, "clGetProgramBuildInfo()");

		assert(status != CL_BUILD_IN_PROGRESS);

		invalid = status == CL_BUILD_ERROR;

		if (invalid)
		{
			cerr << getCompilationLog();
		}
		else
		{
			checkErrorCode(err, CL_SUCCESS, "clBuildProgram()");
		}
	}

	fclose(file);
}

OpenCLProgram::~OpenCLProgram()
{
	clReleaseProgram(program);
}

string OpenCLProgram::getCompilationLog()
{
	size_t logLength;

	cl_int err = clGetProgramBuildInfo(program, clContext->getCLDevice(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLength);
	checkErrorCode(err, CL_SUCCESS, "clGetProgramBuildInfo()");

	char* tmp = new char[logLength + 1];
	tmp[logLength] = 0;

	err = clGetProgramBuildInfo(program, clContext->getCLDevice(), CL_PROGRAM_BUILD_LOG, logLength, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetProgramBuildInfo()");

	string str(tmp);

	delete[] tmp;

	return str;
}

