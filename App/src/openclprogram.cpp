#include "openclprogram.h"

#include "error.h"

#include <cstdlib>
#include <cstdio>

using namespace std;

OpenCLProgram::OpenCLProgram(const char* source, OpenCLContext context)
{
	cl_int err;

	FILE* file = fopen(source, "r");
	checkNotErrorCode(file, nullptr, "File '" << source << "' could not be opened.");

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);

	char* tmp = new char[size + 1];
	tmp[size] = 0;

	rewind(file);
	freadChecked(tmp, sizeof(char), size, file);

	program = clCreateProgramWithSource(context.getCLContext(), 1, const_cast<const char**>(&tmp), &size, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateProgramWithSource()");

	err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clBuildProgram()");

	// to do: print compilation output

	delete[] tmp;
	fclose(file);
}

OpenCLProgram::~OpenCLProgram()
{
	clReleaseProgram(program);
}

