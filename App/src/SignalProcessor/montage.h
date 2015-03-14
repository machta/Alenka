#ifndef MONTAGE_H
#define MONTAGE_H

#include "../openclcontext.h"
#include "../openclprogram.h"

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
		return sources.size();
	}
	static std::string test(const std::string& source, OpenCLContext* context);
	cl_kernel getKernel()
	{
		return program->createKernel("montage");
	}

private:
	std::vector<std::string> sources;
	OpenCLProgram* program;

	static std::string buildSource(const std::vector<std::string>& sources);
	static std::string montageRow(unsigned int row, const std::string& code);
};

#endif // MONTAGE_H
