#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <cassert>

using namespace std;

#define fun() fun_shortcut()

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory, double /*bufferRatio*/) : dataFile(file)
{
	cl_int err;

	M = dataFile->getSamplingFrequency();
	offset = M;
	delay = M/2 - 1;
	padding = 4;
	blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
	dataFileGpuCacheBlockSize = (blockSize + offset)*file->getChannelCount();
	processorTmpBlockSize = (blockSize + offset + padding)*file->getChannelCount();

	// Ensure required sizes.
	if (M%4 || (blockSize + offset)%4)
	{
		throw runtime_error("SignalProcessor requires both the filter length and block length to be multiples of 4");
	}

	clContext = new OpenCLContext(SIGNAL_PROCESSOR_CONTEXT_PARAMETERS, QOpenGLContext::currentContext());

	filterProcessor = new FilterProcessor(M, blockSize + offset, dataFile->getChannelCount(), clContext);

	double Fs = dataFile->getSamplingFrequency();
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

	processorOutputBlockSize = blockSize*montage->getNumberOfRows();

	// Construct the dataFile cache.
	unsigned int dataFileCacheBlockCount = 2*memory/dataFileGpuCacheBlockSize/sizeof(float); // 2* bigger than gpu buffer

	if (dataFileCacheBlockCount <= 0)
	{
		throw runtime_error("Not enough available memory for the dataFileCache");
	}

	dataFileCache.insert(dataFileCache.begin(), dataFileCacheBlockCount, nullptr);
	for (auto& e : dataFileCache)
	{
		e = new float[dataFileGpuCacheBlockSize];
		assert(e != nullptr);
	}

	dataFileCacheLogic = new PriorityCacheLogic(dataFileCacheBlockCount);
	dataFileCacheFillerThread = thread(&SignalProcessor::dataFileCacheFiller, this, &threadsStop);

	// Construct the gpu cache.
	unsigned int gpuCacheBlockCount = memory/dataFileGpuCacheBlockSize/sizeof(float);

	if (gpuCacheBlockCount <= 0)
	{
		throw runtime_error("Not enough available memory for the gpuCache");
	}

	gpuCacheQueue = clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

	gpuCache.insert(gpuCache.begin(), gpuCacheBlockCount, nullptr);
	for (auto& e : gpuCache)
	{
		e = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, dataFileGpuCacheBlockSize*sizeof(float), nullptr, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");
	}

	gpuCacheLogic = new PriorityCacheLogic(gpuCacheBlockCount);
	gpuCacheFillerThread = thread(&SignalProcessor::gpuCacheFiller, this, &threadsStop);

	// Construct processor.
	processorQueuesCount = PROGRAM_OPTIONS["processorQueues"].as<unsigned int>();

	processorQueues.insert(processorQueues.begin(), processorQueuesCount, 0);
	processorTmpBuffers.insert(processorTmpBuffers.begin(), processorQueuesCount, 0);
	processorOutputBuffers.insert(processorOutputBuffers.begin(), processorQueuesCount, 0);
	processorVertexArrays.insert(processorVertexArrays.begin(), processorQueuesCount, 0);

	vector<GLuint> buffers;
	buffers.insert(buffers.begin(), processorQueuesCount, 0);

	fun()->glGenBuffers(processorQueuesCount, buffers.data());
	fun()->glGenVertexArrays(processorQueuesCount, processorVertexArrays.data());

	for (unsigned int i = 0; i < processorQueuesCount; ++i)
	{
		processorQueues[i]  = clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), 0, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

		processorTmpBuffers[i] = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, processorTmpBlockSize*sizeof(float), nullptr, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

		fun()->glBindVertexArray(processorVertexArrays[i]);
		fun()->glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
		fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
		fun()->glEnableVertexAttribArray(0);
		fun()->glBufferData(GL_ARRAY_BUFFER, processorOutputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

		processorOutputBuffers[i] = clCreateFromGLBuffer(clContext->getCLContext(), CL_MEM_WRITE_ONLY, buffers[i], &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateFromGLBuffer()");
	}

	fun()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	fun()->glBindVertexArray(0);
	fun()->glDeleteBuffers(processorQueuesCount, buffers.data());
}

SignalProcessor::~SignalProcessor()
{
	// Join all previously spawned threads.
	threadsStop.store(true);

	{
		thread t([this] () { gpuCacheFillerThread.join(); });

		gpuCacheOutCV.notify_all();

		t.join();
	}

	{
		thread t([this] () { dataFileCacheFillerThread.join(); });

		dataFileCacheOutCV.notify_all();

		t.join();
	}

	// Release resources.
	cl_int err;

	delete clContext;

	delete filterProcessor;
	delete filter;
	delete montageProcessor;
	delete montage;

	delete dataFileCacheLogic;
	delete gpuCacheLogic;

	err = clReleaseCommandQueue(gpuCacheQueue);
	checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");

	for (auto& e : processorQueues)
	{
		err = clReleaseCommandQueue(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");
	}

	for (auto& e : dataFileCache)
	{
		delete[] e;
	}

	for (auto& e : gpuCache)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}

	for (auto& e : processorTmpBuffers)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}

	for (auto& e : processorOutputBuffers)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}

	fun()->glDeleteVertexArrays(processorVertexArrays.size(), processorVertexArrays.data());

	fun();
}

#undef fun

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(indexSet.empty() == false);

	//processorCacheLogic->printInfo();

	cl_int err;
	processorQueuesIndex = (processorQueuesIndex + 1)%processorQueuesCount;
	unsigned int gpuCacheIndex;
	int blockIndex;

	{
		unique_lock<mutex> lock(gpuCacheMutex);

		while (gpuCacheLogic->readAny(indexSet, &gpuCacheIndex, &blockIndex) == false)
		{
			lock.unlock();
			prepareBlocks(indexSet, -1);
			lock.lock();

			fprintf(stderr, "processorInCV(0x%p).wait()\n", &processorInCV);
			processorInCV.wait(lock);
		}
	}

	filterProcessor->process(gpuCache[gpuCacheIndex], processorTmpBuffers[processorQueuesIndex], processorQueues[processorQueuesIndex]);

	cl_event event;

	err = clEnqueueMarkerWithWaitList(processorQueues[processorQueuesIndex], 0, nullptr, &event);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueMarkerWithWaitList()");

	processorQueueCallbackData* data = new processorQueueCallbackData {&gpuCacheMutex, gpuCacheLogic, blockIndex};

	err = clSetEventCallback(event, CL_COMPLETE, &processorQueueCallback, data);
	checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

	err = clEnqueueAcquireGLObjects(processorQueues[processorQueuesIndex], 1, &processorOutputBuffers[processorQueuesIndex], 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffers[processorQueuesIndex], processorOutputBuffers[processorQueuesIndex], processorQueues[processorQueuesIndex]);

	err = clEnqueueReleaseGLObjects(processorQueues[processorQueuesIndex], 1, &processorOutputBuffers[processorQueuesIndex], 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueReleaseGLObjects()");

	err = clFlush(processorQueues[processorQueuesIndex]);
	checkErrorCode(err, CL_SUCCESS, "clFlush()");

	auto fromTo = getBlockBoundaries(blockIndex);
	return SignalBlock(processorVertexArrays[processorQueuesIndex], blockIndex, montage->getNumberOfRows(), fromTo.first, fromTo.second);
}

void SignalProcessor::dataFileCacheFiller(atomic<bool>* stop)
{
	try
	{
		unique_lock<mutex> lock(dataFileCacheMutex);

		while (stop->load() == false)
		{
			unsigned int cacheIndex;
			int blockIndex;

			if (dataFileCacheLogic->fill(&cacheIndex, &blockIndex))
			{
				auto fromTo = getBlockBoundaries(blockIndex);

				dataFile->readData(dataFileCache[cacheIndex], fromTo.first - offset + delay, fromTo.second + delay);

				dataFileCacheLogic->release(blockIndex);

				fprintf(stderr, "gpuCacheInCV(0x%p).notify_one()\n", &gpuCacheInCV);
				gpuCacheInCV.notify_one();
			}
			else
			{
				fprintf(stderr, "dataFileCacheOutCV(0x%p).wait()\n", &dataFileCacheOutCV);
				dataFileCacheOutCV.wait(lock);
			}
		}
	}
	catch (exception& e)
	{
		cerr << "Exception caught in dataFileCacheFiller(): " << e.what() << endl;
		abort();
	}
}

void SignalProcessor::gpuCacheFiller(atomic<bool>* stop)
{
	try
	{
		unique_lock<mutex> gpuCacheLock(gpuCacheMutex);

		while (stop->load() == false)
		{
			unsigned int gpuCacheIndex;
			int blockIndex;

			if (gpuCacheLogic->fill(&gpuCacheIndex, &blockIndex))
			{
				gpuCacheLock.unlock();

				unsigned int dataFileCacheIndex;

				{
					unique_lock<mutex> dataFileCacheLock(dataFileCacheMutex);

					while (dataFileCacheLogic->read(blockIndex, &dataFileCacheIndex) == false)
					{
						fprintf(stderr, "gpuCacheInCV(0x%p).wait()\n", &gpuCacheInCV);
						gpuCacheInCV.wait(dataFileCacheLock);
					}
				}

				gpuCacheLock.lock();

				cl_int err;

				cl_event event;

				err = clEnqueueWriteBuffer(gpuCacheQueue, gpuCache[gpuCacheIndex], CL_FALSE, 0, dataFileGpuCacheBlockSize*sizeof(float), dataFileCache[dataFileCacheIndex], 0, nullptr, &event);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");

				gpuCacheQueueCallbackData* data = new gpuCacheQueueCallbackData {&dataFileCacheMutex, &gpuCacheMutex, dataFileCacheLogic, gpuCacheLogic, &processorInCV, blockIndex};

				err = clSetEventCallback(event, CL_COMPLETE, &gpuCacheQueueCallback, data);
				checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

				err = clFlush(gpuCacheQueue);
				checkErrorCode(err, CL_SUCCESS, "clFlush()");
			}
			else
			{
				fprintf(stderr, "gpuCacheOutCV(0x%p).wait()\n", &gpuCacheOutCV);
				gpuCacheOutCV.wait(gpuCacheLock);
			}
		}
	}
	catch (exception& e)
	{
		cerr << "Exception caught in gpuCacheFiller(): " << e.what() << endl;
		abort();
	}
}

void SignalProcessor::gpuCacheQueueCallback(cl_event event, cl_int event_command_exec_status, void* user_data)
{
	//fprintf(stderr, "gpuCacheQueueCallback()\n");

	assert(event_command_exec_status == CL_COMPLETE);

	gpuCacheQueueCallbackData* data = reinterpret_cast<gpuCacheQueueCallbackData*>(user_data);

	lock_guard<mutex> lock1(*get<0>(*data));
	lock_guard<mutex> lock2(*get<1>(*data));

	int blockIndex = get<5>(*data);
	get<2>(*data)->release(blockIndex);
	get<3>(*data)->release(blockIndex);

	fprintf(stderr, "get<4>(*data)(0x%p)->notify_one()\n", get<4>(*data));
	get<4>(*data)->notify_one();

	cl_int err = clReleaseEvent(event);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	delete data;
}

void SignalProcessor::processorQueueCallback(cl_event event, cl_int event_command_exec_status, void *user_data)
{
	//fprintf(stderr, "processorQueueCallback()\n");

	assert(event_command_exec_status == CL_COMPLETE);

	processorQueueCallbackData* data = reinterpret_cast<processorQueueCallbackData*>(user_data);

	lock_guard<mutex> lock(*get<0>(*data));

	int blockIndex = get<2>(*data);
	get<1>(*data)->release(blockIndex);

	cl_int err = clReleaseEvent(event);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	delete data;
}
