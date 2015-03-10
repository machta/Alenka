#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <cassert>

using namespace std;

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

	// Check block sizes.
	if (M%4 || (blockSize + offset)%4)
	{
		throw runtime_error("SignalProcessor requires both the filter length and block length to be multiples of 4");
	}

	clContext = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS, QOpenGLContext::currentContext());

	// Filter and motage stuff.
	double Fs = file->getSamplingFrequency();
	/*Filter**/ filter = new Filter(M, Fs);
	filter->setHighpass(-100);
	filter->setLowpass(10);
	filter->setNotch(false);

	/*Montage**/ montage = new Montage(vector<string> {"out=in(0);", "out=in(1);", "out=in(2);", "out=(in(0)+in(1)+in(2))/3;", "out=sum(0,2)/3;"}, clContext);
	//montage = new Montage(vector<string> {"out=in(0);", "out=10*in(0);", "out=1000*cos(get_global_id(0)/100.);"}, clContext);

	filterProcessor = new FilterProcessor(M, blockSize + offset, file->getChannelCount(), clContext);
	montageProcessor = new MontageProcessor(offset, blockSize);

	changeFilter(filter);
	changeMontage(montage);

	// Construct the cache.
	unsigned int blockCount = memory/cacheBlockSize/sizeof(float);
	if (blockCount == 0)
	{
		throw runtime_error("Not enough memory for the gpu cache.");
	}

	cache = new GPUCache(blockSize, offset, delay, blockCount, file, clContext);

	// Construct processor.
	processorOutputBlockSize = blockSize*montage->getNumberOfRows();

	processorQueue = clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), 0, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

	GLuint buffer;

	gl()->glGenBuffers(1, &buffer);
	gl()->glGenVertexArrays(1, &processorVertexArray);

	gl()->glBindVertexArray(processorVertexArray);
	gl()->glBindBuffer(GL_ARRAY_BUFFER, buffer);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);
	gl()->glBufferData(GL_ARRAY_BUFFER, processorOutputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

	cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_VERSION_1_2
	flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

	processorTmpBuffer = clCreateBuffer(clContext->getCLContext(), flags, processorTmpBlockSize*sizeof(float), nullptr, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

	flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
	flags = CL_MEM_WRITE_ONLY;
#if CL_VERSION_1_2
	flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

	processorOutputBuffer = clCreateFromGLBuffer(clContext->getCLContext(), flags, buffer, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateFromGLBuffer()");

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl()->glBindVertexArray(0);
	gl()->glDeleteBuffers(1, &buffer);
}

SignalProcessor::~SignalProcessor()
{
	cl_int err;

	delete clContext;

	delete filterProcessor;
	delete montageProcessor;

	delete filter;
	delete montage;

	err = clReleaseCommandQueue(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");

	err = clReleaseMemObject(processorTmpBuffer);
	checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

	err = clReleaseMemObject(processorOutputBuffer);
	checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

	gl()->glDeleteVertexArrays(1, &processorVertexArray);

	gl();
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(indexSet.empty() == false);

	cl_int err;

	cl_event readyEvent = clCreateUserEvent(clContext->getCLContext(), &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

#if CL_VERSION_1_2
	err = clEnqueueBarrierWithWaitList(processorQueue, 1, &readyEvent, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");
#else
	err = clEnqueueWaitForEvents(processorQueue, 1, &readyEvent);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");
#endif

	err = clReleaseEvent(readyEvent);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	printBuffer("after_getAny.txt", processorTmpBuffer, processorQueue);

	filterProcessor->process(processorTmpBuffer, processorQueue);

	printBuffer("after_filter.txt", processorTmpBuffer, processorQueue);

	err = clFlush(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFlush()");

	gl()->glFinish();

	err = clEnqueueAcquireGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, processorQueue);

	printBuffer("after_montage.txt", processorOutputBuffer, processorQueue);

	err = clEnqueueReleaseGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueReleaseGLObjects()");

	err = clFinish(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFinish()");

	auto fromTo = DataFile::getBlockBoundaries(index, getBlockSize());
	return SignalBlock(index, montageProcessor->getNumberOfRows(), fromTo.first, fromTo.second, processorVertexArray);
}
