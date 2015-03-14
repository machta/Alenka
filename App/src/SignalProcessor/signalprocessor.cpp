#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <fstream>

using namespace std;

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory)// : dataFile(file)
{
	cl_int err;

	M = file->getSamplingFrequency();
	offset = M;
	delay = M/2 - 1;
	blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
	cacheBlockSize = (blockSize + offset)*file->getChannelCount();
	processorTmpBlockSize = (blockSize + offset + 4)*file->getChannelCount();

	// Check block sizes.
	if (M%4 || (blockSize + offset)%4)
	{
		throw runtime_error("SignalProcessor requires both the filter length and block length to be multiples of 4");
	}

	clContext = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS, QOpenGLContext::currentContext());

	// Filter and motage stuff.
	double Fs = file->getSamplingFrequency();
	Filter* filter = new Filter(M, Fs);
	filter->setHighpass(PROGRAM_OPTIONS["highpass"].as<double>());
	filter->setLowpass(PROGRAM_OPTIONS["lowpass"].as<double>());
	filter->setNotch(PROGRAM_OPTIONS["notch"].as<bool>());

	vector<string> rows;
	if (PROGRAM_OPTIONS.isSet("montageFile"))
	{
		ifstream mf(PROGRAM_OPTIONS["montageFile"].as<string>());

		while (mf.peek() != EOF)
		{
			string s;
			getline(mf, s);
			rows.push_back(s);
		}
	}
	else
	{
		for (int i = 0; i < file->getChannelCount(); ++i)
		{
			stringstream ss;
			ss << "out = in(" << i << ");";
			rows.push_back(ss.str());
		}
	}
	Montage* montage = new Montage(rows, clContext);

	filterProcessor = new FilterProcessor(M, blockSize + offset, file->getChannelCount(), clContext);
	montageProcessor = new MontageProcessor(offset, blockSize);

	changeFilter(filter);
	delete filter;

	changeMontage(montage);
	delete montage;

	// Construct the cache.
	unsigned int blockCount = memory/cacheBlockSize/sizeof(float);
	if (blockCount == 0)
	{
		throw runtime_error("Not enough memory for the gpu cache.");
	}

	onlineFilter = PROGRAM_OPTIONS["onlineFilter"].as<bool>();

	cache = new GPUCache(blockSize, offset, delay, blockCount, file, clContext, onlineFilter ? nullptr : filterProcessor);

	// Construct processor.
	processorOutputBlockSize = blockSize*montage->getNumberOfRows();

	for (int i = PROGRAM_OPTIONS["commandQueues"].as<unsigned int>(); i >= 0; --i)
	{
		commandQueues.push_back(clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), 0, &err));
		checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");
	}

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

	for (const auto& e : commandQueues)
	{
		err = clReleaseCommandQueue(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");
	}

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
	err = clEnqueueBarrierWithWaitList(commandQueues[0], 1, &readyEvent, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");
#else
	err = clEnqueueWaitForEvents(commandQueues[0], 1, &readyEvent);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueWaitForEvents()");
#endif

	err = clReleaseEvent(readyEvent);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	printBuffer("after_getAny.txt", processorTmpBuffer, commandQueues[0]);

	if (onlineFilter)
	{
		filterProcessor->process(processorTmpBuffer, commandQueues[0]);

		printBuffer("after_filter.txt", processorTmpBuffer, commandQueues[0]);

		err = clFlush(commandQueues[0]);
		checkErrorCode(err, CL_SUCCESS, "clFlush()");
	}

	cl_event event;

#if CL_VERSION_1_2
	err = clEnqueueMarkerWithWaitList(commandQueues[0], 0, nullptr, &event);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueMarkerWithWaitList()");

	for (const auto& e : commandQueues)
	{
		err = clEnqueueBarrierWithWaitList(e, 1, &event, nullptr);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");
	}
#else
	err = clEnqueueMarker(commandQueues[0], &event);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueMarker()");

	for (const auto& e : commandQueues)
	{
		err = clEnqueueWaitForEvents(e, 1, &event);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueWaitForEvents()");
	}
#endif

	err = clReleaseEvent(event);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	gl()->glFinish();

	err = clEnqueueAcquireGLObjects(commandQueues[0], 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, commandQueues);

	printBuffer("after_montage.txt", processorOutputBuffer, commandQueues[0]);

	for (const auto& e : commandQueues)
	{
		err = clFinish(e);
		checkErrorCode(err, CL_SUCCESS, "clFinish()");
	}

	err = clEnqueueReleaseGLObjects(commandQueues[0], 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueReleaseGLObjects()");

	auto fromTo = DataFile::getBlockBoundaries(index, getBlockSize());
	return SignalBlock(index, montageProcessor->getNumberOfRows(), fromTo.first, fromTo.second, processorVertexArray);
}
