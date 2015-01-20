#include "program.h"

#include <cstdio>

#include <stdexcept>

using namespace std;

Program::Program(const char* source, Context context)
{
	cl_int err;

	FILE* file = fopen(source, "r");
	if (file == nullptr)
	{
		stringstream ss;
		ss << "File '" << source << "' could not be opened.";
		throw runtime_error(ss.str());
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);

	char* tmp = new char[size + 1];
	tmp[size] = 0;

	rewind(file);
	fread(tmp, sizeof(char), size, file);

	program = clCreateProgramWithSource(context.getCLContext(), 1, const_cast<const char**>(&tmp), &size, &err);
	if (err != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clCreateProgramWithSource returned with error code " << err << ".";
		throw runtime_error(ss.str());
	}

	err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clBuildProgram returned with error code " << err << ".";
		throw runtime_error(ss.str());
	}

	// to do: print compilation output

	delete[] tmp;
	fclose(file);
}

Program::~Program()
{
	clReleaseProgram(program);
}

