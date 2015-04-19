#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <fstream>

using namespace std;

SignalProcessor::SignalProcessor()
{
	cl_int err;

	context = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS, QOpenGLContext::currentContext());

	commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
	checkClErrorCode(err, "clCreateCommandQueue()");

	gl()->glGenBuffers(1, &glBuffer);
	gl()->glGenVertexArrays(2, vertexArrays);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);

	gl()->glBindVertexArray(vertexArrays[0]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindVertexArray(vertexArrays[1]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2*sizeof(float), reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl()->glBindVertexArray(0);
}

SignalProcessor::~SignalProcessor()
{
	cl_int err;

	destroyFileRelated();

	delete context;

	err = clReleaseCommandQueue(commandQueue);
	checkClErrorCode(err, "clReleaseCommandQueue()");

	gl()->glDeleteBuffers(1, &glBuffer);
	gl()->glDeleteVertexArrays(2, vertexArrays);

	gl();
}

void SignalProcessor::updateFilter()
{
	using namespace std;

	if (file == nullptr)
	{
		return;
	}

	Filter filter(static_cast<unsigned int>(file->getSamplingFrequency()), file->getSamplingFrequency()); // Possibly could save this object so that it won't be created from scratch everytime.
	filter.setLowpass(getInfoTable()->getLowpassFrequency());
	filter.setHighpass(getInfoTable()->getHighpassFrequency());
	filter.setNotch(getInfoTable()->getNotch());

	if (PROGRAM_OPTIONS.isSet("printFilter"))
	{
		if (PROGRAM_OPTIONS.isSet("printFilterFile"))
		{
			FILE* file = fopen(PROGRAM_OPTIONS["printFilterFile"].as<string>().c_str(), "w");
			checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["printFilterFile"].as<string>() << "' could not be opened for wtiting.");

			filter.printCoefficients(file);

			fclose(file);
		}
		else
		{
			filter.printCoefficients(stderr);
		}
	}

	filterProcessor->change(&filter);

	if (onlineFilter == false)
	{
		cache->clear();
	}
}

void SignalProcessor::updateMontage()
{
	if (file == nullptr)
	{
		return;
	}

	TrackTable* tt = file->getMontageTable()->getTrackTables()->at(getInfoTable()->getSelectedMontage());

	auto code = tt->getCode();

	if ((trackCount = code.size()) == 0)
	{
		return;
	}

	assert(ready());

	Montage montage(code, context);

	assert(montage.getNumberOfRows() > 0);

	montageProcessor->change(&montage);

	releaseOutputBuffer();

	gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);

	unsigned int outputBlockSize = blockSize*trackCount;
	outputBlockSize *= PROGRAM_OPTIONS["eventRenderMode"].as<int>();

	gl()->glBufferData(GL_ARRAY_BUFFER, outputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

	cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
	flags = CL_MEM_WRITE_ONLY;
#if CL_VERSION_1_2
	flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

	cl_int err;

	processorOutputBuffer = clCreateFromGLBuffer(context->getCLContext(), flags, glBuffer, &err);
	checkClErrorCode(err, "clCreateFromGLBuffer()");

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(ready());
	assert(indexSet.empty() == false);

	cl_int err;

	cl_event readyEvent = clCreateUserEvent(context->getCLContext(), &err);
	checkClErrorCode(err, "clCreateUserEvent()");

#if CL_VERSION_1_2
	err = clEnqueueBarrierWithWaitList(commandQueue, 1, &readyEvent, nullptr);
	checkClErrorCode(err, "clEnqueueBarrierWithWaitList()");
#else
	err = clEnqueueWaitForEvents(commandQueue, 1, &readyEvent);
	checkClErrorCode(err, "clEnqueueWaitForEvents()");
#endif

	err = clReleaseEvent(readyEvent);
	checkClErrorCode(err, "clReleaseEvent()");

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	printBuffer("after_getAny.txt", processorTmpBuffer, commandQueue);

	if (onlineFilter)
	{
		filterProcessor->process(processorTmpBuffer, commandQueue);

		printBuffer("after_filter.txt", processorTmpBuffer, commandQueue);

		err = clFlush(commandQueue);
		checkClErrorCode(err, "clFlush()");
	}

	gl()->glFinish(); // Could be replaced by a fence.

	err = clEnqueueAcquireGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, commandQueue);

	printBuffer("after_montage.txt", processorOutputBuffer, commandQueue);

	err = clFinish(commandQueue);
	checkClErrorCode(err, "clFinish()");

	err = clEnqueueReleaseGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkClErrorCode(err, "clEnqueueReleaseGLObjects()");

	err = clFinish(commandQueue);
	checkClErrorCode(err, "clFinish()");

	auto fromTo = DataFile::blockIndexToSampleRange(index, getBlockSize());
	return SignalBlock(index, fromTo.first, fromTo.second, vertexArrays);
}

void SignalProcessor::changeFile(DataFile* file)
{
	destroyFileRelated();

	this->file = file;

	if (file == nullptr)
	{
		infoTable = nullptr;
	}
	else
	{
		infoTable = file->getInfoTable();

		int M = file->getSamplingFrequency();
		int offset = M;
		int delay = M/2 - 1;
		blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
		unsigned int tmpBlockSize = (blockSize + offset + 4)*file->getChannelCount();

		// Check block sizes.
		if (M%4 || (blockSize + offset)%4)
		{
			throw runtime_error("SignalProcessor requires both the filter length and block length to be multiples of 4");
		}

		// Construct the filter and montage processors.
		filterProcessor = new FilterProcessor(M, blockSize + offset, file->getChannelCount(), context);

		montageProcessor = new MontageProcessor(offset, blockSize, file->getChannelCount());

		// Construct the cache.
		onlineFilter = PROGRAM_OPTIONS["onlineFilter"].as<bool>();

		int64_t memory = PROGRAM_OPTIONS["gpuMemorySize"].as<int64_t>();
		cl_int err;

		if (memory <= 0)
		{
			cl_ulong size;

			err = clGetDeviceInfo(context->getCLDevice(), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &size, nullptr);
			checkClErrorCode(err, "clGetDeviceInfo()");

			memory += size;
		}

		memory -= tmpBlockSize + blockSize*sizeof(float)*500; // substract the sizes of the tmp buffer and the output buffer (for a realistically big montage)

		cache = new GPUCache(blockSize, offset, delay, memory, file, context, onlineFilter ? nullptr : filterProcessor);

		// Construct tmp buffer.
		cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_VERSION_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

		processorTmpBuffer = clCreateBuffer(context->getCLContext(), flags, tmpBlockSize*sizeof(float), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer()");

		// Default filter and montage.
		updateFilter();

		updateMontage();
	}
}

void SignalProcessor::destroyFileRelated()
{
	if (file != nullptr)
	{
		file = nullptr;

		delete cache;
		delete filterProcessor;
		delete montageProcessor;

		cl_int err = clReleaseMemObject(processorTmpBuffer);
		checkClErrorCode(err, "clReleaseMemObject()");

		releaseOutputBuffer();
	}
}
