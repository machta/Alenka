#include "openclprogram.h"

#include "error.h"

#include <cstdlib>
#include <iostream>
#include <cassert>

using namespace std;

OpenCLProgram::~OpenCLProgram()
{
	cl_int err = clReleaseProgram(program);
	checkErrorCode(err, CL_SUCCESS, "clReleaseProgram()");
}

string OpenCLProgram::getCompilationLog() const
{
	size_t logLength;

	cl_int err = clGetProgramBuildInfo(program, context->getCLDevice(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLength);
	checkErrorCode(err, CL_SUCCESS, "clGetProgramBuildInfo()");

	char* tmp = new char[logLength + 1];
	tmp[logLength] = 0;

	err = clGetProgramBuildInfo(program, context->getCLDevice(), CL_PROGRAM_BUILD_LOG, logLength, tmp, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clGetProgramBuildInfo()");

	string str(tmp);

	delete[] tmp;

	return str;
}

void OpenCLProgram::construct(const string& source)
{
	cl_int err;

	const char* sourcePointer = source.c_str();
	size_t size = source.size();

	program = clCreateProgramWithSource(context->getCLContext(), 1, &sourcePointer, &size, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateProgramWithSource()");

	err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

	if (err == CL_SUCCESS)
	{
		invalid = false;
	}
	else
	{
		cl_build_status status;

		cl_int err2 = clGetProgramBuildInfo(program, context->getCLDevice(), CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, nullptr);
		checkErrorCode(err2, CL_SUCCESS, "clGetProgramBuildInfo()");

		assert(status != CL_BUILD_IN_PROGRESS);

		invalid = status == CL_BUILD_ERROR;

		if (invalid)
		{
			logToBoth(getCompilationLog());
		}
		else
		{
			checkErrorCode(err, CL_SUCCESS, "clBuildProgram()");
		}
	}
}

