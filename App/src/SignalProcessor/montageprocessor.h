#ifndef MONTAGEPROCESSOR_H
#define MONTAGEPROCESSOR_H

#include "montage.h"
#include "../openclprogram.h"

#include <CL/cl_gl.h>
#include <clFFT.h>

#include <vector>

class MontageProcessor
{
public:
	MontageProcessor(unsigned int offset, unsigned int blockWidth);
	~MontageProcessor();

	void change(Montage* montage);
	void process(cl_mem inBuffer, cl_mem outBuffer, const std::vector<cl_command_queue>& queues);
	unsigned int getNumberOfRows() const
	{
		return montageKernels.size();
	}

private:
	cl_int inputRowLength;
	cl_int inputRowOffset;
	cl_int outputRowLength;
	OpenCLProgram* montageProgram = nullptr;
	std::vector<cl_kernel> montageKernels;

	void releaseMontage()
	{
		cl_int err;

		delete montageProgram;

		for (const auto& e : montageKernels)
		{
			err = clReleaseKernel(e);
			checkErrorCode(err, CL_SUCCESS, "clReleaseKernel()");
		}

		montageKernels.clear();
	}
};

#endif // MONTAGEPROCESSOR_H
