#ifndef FILTERPROCESSOR_H
#define FILTERPROCESSOR_H

#include "filter.h"
#include "../openclcontext.h"
#include "../openclprogram.h"

#include <clFFT.h>

#include <vector>

/**
 * @brief This class handles filtering of data blocks.
 */
class FilterProcessor
{
public:
	/**
	 * @brief FilterProcessor constructor.
	 * @param M The size of the filter.
	 */
	FilterProcessor(unsigned int M, unsigned int blockWidth, unsigned int blockHeight, OpenCLContext* context);
	~FilterProcessor();

	/**
	 * @brief From now on use coefficients of filter for computations.
	 */
	void change(Filter* filter);
	
	/**
	 * @brief Enqueues all commands required for filter computation to queue.
	 * @param buffer [in,out]
	 */
	void process(cl_mem buffer, cl_command_queue queue);

private:
	unsigned int M;
	unsigned int width;
	unsigned int height;
	OpenCLProgram* program;
	cl_kernel filterKernel;
	cl_kernel zeroKernel;
	cl_mem filterBuffer;
	bool coefficientsChanged = false;
	std::vector<float> coefficients;
	clfftPlanHandle fftPlan;
	clfftPlanHandle fftPlanBatch;
	clfftPlanHandle ifftPlanBatch;
};

#endif // FILTERPROCESSOR_H
