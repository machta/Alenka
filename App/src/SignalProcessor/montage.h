#ifndef MONTAGE_H
#define MONTAGE_H

#include "../openclcontext.h"
#include "../openclprogram.h"
#include "../error.h"

#include <CL/cl_gl.h>

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
	cl_kernel getKernel()
	{
		return program->createKernel("montage");
	}

	static std::string test(const std::string& source, OpenCLContext* context);

private:
	unsigned int numberOfRows;
	OpenCLProgram* program;

	static std::string buildSource(const std::vector<std::string>& sources);
	static std::string montageRow(unsigned int row, const std::string& code);
};

#endif // MONTAGE_H
