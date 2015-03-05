#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <cassert>

using namespace std;

#define fun() fun_shortcut()

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory)// : dataFile(file)
{
	cl_int err;

	M = file->getSamplingFrequency();
	offset = M;
	delay = M/2 - 1;
	padding = 4;
	blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
	cacheBlockSize = (blockSize + offset)*file->getChannelCount();
	processorTmpBlockSize = (blockSize + offset + padding)*file->getChannelCount();

	// Ensure required sizes.
	if (M%4 || (blockSize + offset)%4)
	{
		throw runtime_error("SignalProcessor requires both the filter length and block length to be multiples of 4");
	}

	clContext = new OpenCLContext(SIGNAL_PROCESSOR_CONTEXT_PARAMETERS, QOpenGLContext::currentContext());

	// Filter and motage stuff.
	filterProcessor = new FilterProcessor(M, blockSize + offset, file->getChannelCount(), clContext);

	double Fs = file->getSamplingFrequency();
	filter = new Filter(M, Fs);
	filter->setHighpass(0);
	filter->setLowpass(Fs/4);
	filter->setNotch(false);

	if (PROGRAM_OPTIONS.isSet("printFilter"))
	{
		if (PROGRAM_OPTIONS.isSet("printFilterFile"))
		{
			FILE* file = fopen(PROGRAM_OPTIONS["printFilterFile"].as<string>().c_str(), "w");
			checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["printFilterFile"].as<string>() << "' could not be opened for wtiting.");

			filter->printCoefficients(file);

			fclose(file);
		}
		else
		{
			filter->printCoefficients(stderr);
		}
	}

	filterProcessor->change(filter);

	montage = new Montage(vector<string> {"out=in(0);", "out=in(1);", "out=in(2);", "out=(in(0)+in(1)+in(2))/3;", "out=sum(0,2)/3;"}, clContext);
	//montage = new Montage(vector<string> {"out=in(0);", "out=10*in(0);", "out=1000*cos(get_global_id(0)/100.);"}, clContext);
	montageProcessor = new MontageProcessor(offset, blockSize);
	montageProcessor->change(montage);

	// Construct the cache.
	unsigned int blockCount = memory/cacheBlockSize/sizeof(float);
	if (blockCount == 0)
	{
		throw runtime_error("Not enough memory for the gpu cache.");
	}

	cache = new GPUCache(blockSize, offset, delay, blockCount, file, clContext);

	// Construct processor.
	processorOutputBlockSize = blockSize*montage->getNumberOfRows();

	processorQueue  = clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), 0, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

	GLuint buffer;

	fun()->glGenBuffers(1, &buffer);
	fun()->glGenVertexArrays(1, &processorVertexArray);

	fun()->glBindVertexArray(processorVertexArray);
	fun()->glBindBuffer(GL_ARRAY_BUFFER, buffer);
	fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	fun()->glEnableVertexAttribArray(0);
	fun()->glBufferData(GL_ARRAY_BUFFER, processorOutputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

	processorTmpBuffer = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, processorTmpBlockSize*sizeof(float), nullptr, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

	processorOutputBuffer = clCreateFromGLBuffer(clContext->getCLContext(), CL_MEM_WRITE_ONLY, buffer, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateFromGLBuffer()");

	fun()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	fun()->glBindVertexArray(0);
	fun()->glDeleteBuffers(1, &buffer);
}

SignalProcessor::~SignalProcessor()
{
	// Release resources.
	cl_int err;

	delete clContext;

	delete filterProcessor;
	delete filter;
	delete montageProcessor;
	delete montage;

	err = clReleaseCommandQueue(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");

	err = clReleaseMemObject(processorTmpBuffer);
	checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

	err = clReleaseMemObject(processorOutputBuffer);
	checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

	fun()->glDeleteVertexArrays(1, &processorVertexArray);

	fun();
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(indexSet.empty() == false);

	cl_int err;

	cl_event readyEvent = clCreateUserEvent(clContext->getCLContext(), &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

	cerr << "Create event " << readyEvent <<  "(" << &readyEvent << ")" << endl;

	err = clEnqueueBarrierWithWaitList(processorQueue, 1, &readyEvent, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");

	err = clReleaseEvent(readyEvent);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	filterProcessor->process(processorTmpBuffer, processorQueue);

	err = clFlush(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFlush()");

	fun()->glFinish();

	err = clEnqueueAcquireGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, processorQueue);

	err = clEnqueueReleaseGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueReleaseGLObjects()");

	err = clFinish(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFinish()");

	auto fromTo = DataFile::getBlockBoundaries(index, cacheBlockSize);
	return SignalBlock(index, montage->getNumberOfRows(), fromTo.first, fromTo.second, processorVertexArray);
}

#undef fun
