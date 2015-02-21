#ifndef FILTERPROCESSOR_H
#define FILTERPROCESSOR_H

#include "filter.h"
#include "../openclcontext.h"
#include "../openclprogram.h"

#include <clFFT.h>

class FilterProcessor
{
public:
	FilterProcessor(unsigned int M, unsigned int blockWidth, unsigned int blockHeight, OpenCLContext* context);
	~FilterProcessor();

	void change(Filter* filter);
	void process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue);

private:
	unsigned int M;
	unsigned int width;
	unsigned int height;
	OpenCLProgram* program;
	cl_kernel filterKernel;
	cl_mem filterBuffer;
	bool coefficientsChanged = false;
	float* coefficients;
	clfftPlanHandle fftPlan;
	clfftPlanHandle fftPlanBatch;
	clfftPlanHandle ifftPlanBatch;
};

#endif // FILTERPROCESSOR_H
