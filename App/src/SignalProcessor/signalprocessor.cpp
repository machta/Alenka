#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>

using namespace std;

SignalProcessor::SignalProcessor()
{
	onlineFilter = PROGRAM_OPTIONS["onlineFilter"].as<bool>();
	glSharing = PROGRAM_OPTIONS["glSharing"].as<bool>();

	cl_int err;

	initializeOpenGLInterface();

	if (glSharing)
	{
		context = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS, QOpenGLContext::currentContext());
	}
	else
	{
		context = globalContext.get();
	}

	commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
	checkClErrorCode(err, "clCreateCommandQueue()");

	gl()->glGenBuffers(1, &glBuffer);
	glGenVertexArrays(2, vertexArrays);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);

	glBindVertexArray(vertexArrays[0]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	glBindVertexArray(vertexArrays[1]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2*sizeof(float), reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	gl();
}

SignalProcessor::~SignalProcessor()
{
	cl_int err;

	destroyFileRelated();

	delete context;

	err = clReleaseCommandQueue(commandQueue);
	checkClErrorCode(err, "clReleaseCommandQueue()");

	gl()->glDeleteBuffers(1, &glBuffer);
	glDeleteVertexArrays(2, vertexArrays);

	gl();
}

void SignalProcessor::updateFilter()
{
	using namespace std;

	if (file == nullptr)
	{
		return;
	}

	Filter filter(context, static_cast<unsigned int>(file->getSamplingFrequency()), file->getSamplingFrequency()); // Possibly could save this object so that it won't be created from scratch everytime.
	filter.setLowpass(getInfoTable()->getLowpassFrequency());
	filter.setHighpass(getInfoTable()->getHighpassFrequency());
	filter.setNotch(getInfoTable()->getNotch());

	if (PROGRAM_OPTIONS.isSet("printFilter"))
	{
		if (PROGRAM_OPTIONS.isSet("printFilterFile"))
		{
			FILE* file = fopen(PROGRAM_OPTIONS["printFilterFile"].as<string>().c_str(), "w");
			checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["printFilterFile"].as<string>() << "' could not be opened for writing.");

			filter.printCoefficients(file);

			int err = fclose(file);
			checkErrorCode(err, 0, "fclose()");
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

void SignalProcessor::setUpdateMontageFlag()
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

	updateMontageFlag = true;
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(ready());
	assert(indexSet.empty() == false);

	if (updateMontageFlag)
	{
		updateMontageFlag = false;
		updateMontage();
	}

	cl_int err;

	cl_event readyEvent = clCreateUserEvent(context->getCLContext(), &err);
	checkClErrorCode(err, "clCreateUserEvent()");

	OpenCLContext::enqueueBarrier(commandQueue, readyEvent);

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	printBuffer("after_getAny.txt", processorTmpBuffer, commandQueue);

	if (onlineFilter)
	{
		filterProcessor->process(processorTmpBuffer, commandQueue);

		printBuffer("after_filter.txt", processorTmpBuffer, commandQueue);

		err = clFlush(commandQueue);
		checkClErrorCode(err, "clFlush()");
	}

	if (glSharing)
	{
		gl()->glFinish(); // Could be replaced by a fence.

		err = clEnqueueAcquireGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueAcquireGLObjects()");
	}

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, commandQueue);

	printBuffer("after_montage.txt", processorOutputBuffer, commandQueue);

	if (glSharing)
	{
		err = clFinish(commandQueue);
		checkClErrorCode(err, "clFinish()");

		err = clEnqueueReleaseGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReleaseGLObjects()");

		err = clFinish(commandQueue);
		checkClErrorCode(err, "clFinish()");
	}
	else
	{
		// Pull the data from CL buffer and copy it to the GL buffer.

		unsigned int outputBlockSize = blockSize*trackCount; // Code-duplicity.
		outputBlockSize *= PROGRAM_OPTIONS["eventRenderMode"].as<int>();

		err = clEnqueueReadBuffer(commandQueue, processorOutputBuffer, CL_TRUE, 0, outputBlockSize*sizeof(float), processorOutputBufferTmp, 0, nullptr, nullptr);
		checkClErrorCode(err, "clGetMemObjectInfo");

		gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		gl()->glBufferData(GL_ARRAY_BUFFER, outputBlockSize*sizeof(float), processorOutputBufferTmp, GL_STATIC_DRAW);
	}

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
		int64_t memoryToUse = PROGRAM_OPTIONS["gpuMemorySize"].as<int64_t>();
		cl_int err;

		cl_ulong maxMemorySize;
		err = clGetDeviceInfo(context->getCLDevice(), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &maxMemorySize, nullptr);
		checkClErrorCode(err, "clGetDeviceInfo()");

		if (memoryToUse <= 0)
			memoryToUse += maxMemorySize;

		// Ensure the limit is within a reasonable interval.
		memoryToUse = max<int64_t>(memoryToUse, maxMemorySize*0.1);
		memoryToUse = min<int64_t>(memoryToUse, maxMemorySize*0.75);

		memoryToUse -= tmpBlockSize + blockSize*sizeof(float)*2048; // substract the sizes of the tmp buffer and the output buffer (for a realistically big montage)
		assert(memoryToUse > 0 && "There is no memory left for the GPU buffer.");

		cache = new GPUCache(blockSize, offset, delay, memoryToUse, file, context, onlineFilter ? nullptr : filterProcessor);

		// Construct tmp buffer.
		cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

		processorTmpBuffer = clCreateBuffer(context->getCLContext(), flags, tmpBlockSize*sizeof(float), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer()");

		// Default filter and montage.
		updateFilter();

		setUpdateMontageFlag();
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

void SignalProcessor::updateMontage()
{
	assert(ready());

	auto code = file->getMontageTable()->getTrackTables()->at(getInfoTable()->getSelectedMontage())->getCode();

	Montage montage(code, context);

	assert(montage.getNumberOfRows() > 0);

	montageProcessor->change(&montage);

	releaseOutputBuffer();

	unsigned int outputBlockSize = blockSize*trackCount; // Code-duplicity.
	outputBlockSize *= PROGRAM_OPTIONS["eventRenderMode"].as<int>();

	cl_int err;

	if (glSharing)
	{
		gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		gl()->glBufferData(GL_ARRAY_BUFFER, outputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

		cl_mem_flags flags = CL_MEM_READ_WRITE;
	#ifdef NDEBUG
		flags = CL_MEM_WRITE_ONLY;
	#if CL_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
	#endif
	#endif

		processorOutputBuffer = clCreateFromGLBuffer(context->getCLContext(), flags, glBuffer, &err);
		checkClErrorCode(err, "clCreateFromGLBuffer()");
	}
	else
	{
		cl_mem_flags flags = CL_MEM_READ_WRITE;
	#ifdef NDEBUG
	#if CL_1_2
		flags |= CL_MEM_HOST_READ_ONLY;
	#endif
	#endif

		processorOutputBuffer = clCreateBuffer(context->getCLContext(), flags, outputBlockSize*sizeof(float), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		assert(processorOutputBufferTmp == nullptr && "Make sure the buffer does't leak.");
		processorOutputBufferTmp = new float[outputBlockSize];
	}

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
}
