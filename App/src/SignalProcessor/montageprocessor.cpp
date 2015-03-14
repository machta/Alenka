#include "montageprocessor.h"

#include "../error.h"

using namespace std;

MontageProcessor::MontageProcessor(unsigned int offset, unsigned int blockWidth) :
	inputRowLength((offset + blockWidth + 4)/4), inputRowOffset(offset/4), outputRowLength(blockWidth/4)
{

}

MontageProcessor::~MontageProcessor()
{
	releaseMontage();
}

void MontageProcessor::change(Montage* montage)
{
	releaseMontage();

	montageProgram = montage->getProgram();

	for (unsigned int i = 0; i < montage->getNumberOfRows(); ++i)
	{
		stringstream ss;
		ss << "montage" << i;

		montageKernels.push_back(montageProgram->createKernel(ss.str()));
	}
}

void MontageProcessor::process(cl_mem inBuffer, cl_mem outBuffer, const vector<cl_command_queue>& queues)
{
	cl_int err;

	unsigned int queueIndex = 0;

	for (unsigned int i = 0; i < getNumberOfRows(); ++i)
	{
		err = clSetKernelArg(montageKernels[i], 0, sizeof(cl_mem), &inBuffer);
		checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

		err = clSetKernelArg(montageKernels[i], 1, sizeof(cl_mem), &outBuffer);
		checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

		err = clSetKernelArg(montageKernels[i], 2, sizeof(cl_int), &inputRowLength);
		checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

		err = clSetKernelArg(montageKernels[i], 3, sizeof(cl_int), &inputRowOffset);
		checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

		cl_int outputOffset = outputRowLength*i;
		err = clSetKernelArg(montageKernels[i], 4, sizeof(cl_int), &outputOffset);
		checkErrorCode(err, CL_SUCCESS, "clSetKernelArg()");

		size_t globalWorkSize = outputRowLength;

		err = clEnqueueNDRangeKernel(queues[queueIndex], montageKernels[i], 1, nullptr, &globalWorkSize, nullptr, 0, nullptr, nullptr);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueNDRangeKernel()");

		queueIndex = (queueIndex + 1)%queues.size();
	}
}
