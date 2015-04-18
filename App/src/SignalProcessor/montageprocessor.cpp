#include "montageprocessor.h"

#include "../error.h"

using namespace std;

MontageProcessor::MontageProcessor(unsigned int offset, unsigned int blockWidth, int channelsInFile) :
	inputRowLength((offset + blockWidth + 4)/4), inputRowOffset(offset/4), outputRowLength(blockWidth/4), channelsInFile(channelsInFile)
{
}

MontageProcessor::~MontageProcessor()
{
	releaseMontage();
}

void MontageProcessor::change(Montage* montage)
{
	releaseMontage();
	montageKernel = montage->getKernel();
	numberOfRows = montage->getNumberOfRows();
}

void MontageProcessor::process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue)
{
	cl_int err;

	err = clSetKernelArg(montageKernel, 0, sizeof(cl_mem), &inBuffer);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(montageKernel, 1, sizeof(cl_mem), &outBuffer);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(montageKernel, 2, sizeof(cl_int), &inputRowLength);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(montageKernel, 3, sizeof(cl_int), &inputRowOffset);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(montageKernel, 4, sizeof(cl_int), &outputRowLength);
	checkClErrorCode(err, "clSetKernelArg()");

	err = clSetKernelArg(montageKernel, 5, sizeof(cl_int), &channelsInFile);
	checkClErrorCode(err, "clSetKernelArg()");

	size_t globalWorkSize = outputRowLength;

	err = clEnqueueNDRangeKernel(queue, montageKernel, 1, nullptr, &globalWorkSize, nullptr, 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueNDRangeKernel()");
}
