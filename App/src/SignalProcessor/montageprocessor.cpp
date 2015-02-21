#include "montageprocessor.h"

#include "../error.h"

MontageProcessor::MontageProcessor(unsigned int offset, unsigned int blockWidth) :
	inputRowLength((offset + blockWidth + 4)/4), inputRowOffset(offset/4), outputRowLength(blockWidth/4)
{

}

MontageProcessor::~MontageProcessor()
{

}

void MontageProcessor::change(Montage* montage)
{
	this->montage = montage;
}

void MontageProcessor::process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue)
{
	cl_int err;

	err = clSetKernelArg(montage->getKernel(), 0, sizeof(cl_mem), &inBuffer);
	checkNotErrorCode(err, CL_SUCCESS, "clSetKernelArg");

	err = clSetKernelArg(montage->getKernel(), 1, sizeof(cl_mem), &outBuffer);
	checkNotErrorCode(err, CL_SUCCESS, "clSetKernelArg");

	err = clSetKernelArg(montage->getKernel(), 2, sizeof(cl_int), &inputRowLength);
	checkNotErrorCode(err, CL_SUCCESS, "clSetKernelArg");

	err = clSetKernelArg(montage->getKernel(), 3, sizeof(cl_int), &inputRowOffset);
	checkNotErrorCode(err, CL_SUCCESS, "clSetKernelArg");

	err = clSetKernelArg(montage->getKernel(), 4, sizeof(cl_int), &outputRowLength);
	checkNotErrorCode(err, CL_SUCCESS, "clSetKernelArg");

	size_t globalWorkSize = outputRowLength;

	err = clEnqueueNDRangeKernel(queue, montage->getKernel(), 1, nullptr, &globalWorkSize, nullptr, 0, nullptr, nullptr);
	checkNotErrorCode(err, CL_SUCCESS, "clEnqueueNDRangeKernel");
}

