#ifndef MONTAGEPROCESSOR_H
#define MONTAGEPROCESSOR_H

#include "montage.h"

#include <clFFT.h>

class MontageProcessor
{
public:
	MontageProcessor(unsigned int offset, unsigned int blockWidth);
	~MontageProcessor();

	void change(Montage* montage);
	void process(cl_mem inBuffer, cl_mem outBuffer, cl_command_queue queue);

private:
	cl_int inputRowLength;
	cl_int inputRowOffset;
	cl_int outputRowLength;
	Montage* montage;
};

#endif // MONTAGEPROCESSOR_H
