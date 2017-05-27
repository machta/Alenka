#include "../include/AlenkaSignal/montageprocessor.h"

#include "../include/AlenkaSignal/openclcontext.h"
#include "../include/AlenkaSignal/montage.h"

using namespace std;

namespace AlenkaSignal
{

template<class T>
void MontageProcessor<T>::process(const vector<Montage<T>*>& montage, cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue, cl_int outputRowLength, cl_int inputRowOffset)
{
	cl_int err;
	size_t inSize, outSize;

	err = clGetMemObjectInfo(inBuffer, CL_MEM_SIZE, sizeof(size_t), &inSize, nullptr);
	checkClErrorCode(err, "clGetMemObjectInfo");

	if (inSize < inputRowLength*inputRowCount*sizeof(T))
		throw runtime_error("MontageProcessor: the inBuffer is too small.");

	err = clGetMemObjectInfo(outBuffer, CL_MEM_SIZE, sizeof(size_t), &outSize, nullptr);
	checkClErrorCode(err, "clGetMemObjectInfo");

	if (outSize < outputRowLength*montage.size()*outputCopyCount*sizeof(T))
		throw runtime_error("MontageProcessor: the outBuffer is too small.");

	for (unsigned int i = 0; i < montage.size(); i++)
	{
		cl_kernel montageKernel = montage[i]->getKernel();
		int pi = 0;

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_mem), &inBuffer);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_mem), &outBuffer);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &inputRowLength);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &inputRowOffset);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &inputRowCount);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &outputRowLength);
		checkClErrorCode(err, "clSetKernelArg()");

		cl_int index = i;
		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &index);
		checkClErrorCode(err, "clSetKernelArg()");

		err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &outputCopyCount);
		checkClErrorCode(err, "clSetKernelArg()");

		if (montage[i]->isCopyMontage())
		{
			cl_int copyIndex = montage[i]->copyMontageIndex();
			err = clSetKernelArg(montageKernel, pi++, sizeof(cl_int), &copyIndex);
			checkClErrorCode(err, "clSetKernelArg()");
		}

		size_t globalWorkSize = outputRowLength;

		err = clEnqueueNDRangeKernel(queue, montageKernel, 1, nullptr, &globalWorkSize, nullptr, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueNDRangeKernel()");
	}
}

template class MontageProcessor<float>;
template class MontageProcessor<double>;

} // namespace AlenkaSignal
