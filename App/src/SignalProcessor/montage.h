#ifndef MONTAGE_H
#define MONTAGE_H

#include "../openclcontext.h"
#include "../openclprogram.h"

#include <CL/cl.h>

#include <vector>
#include <string>

class Montage
{
public:
	Montage(const std::vector<std::string>& sources, OpenCLContext* context);
	~Montage();

	unsigned int getNumberOfRows() const
	{
		return numberOfRows;
	}
	static std::string test(const std::string& source, OpenCLContext* context);
	cl_kernel getKernel() const
	{
		return kernel;
	}

private:
	unsigned int numberOfRows;
	OpenCLProgram* program;
	cl_kernel kernel;

	static std::string buildSource(const std::vector<std::string>& sources);
};

#endif // MONTAGE_H
