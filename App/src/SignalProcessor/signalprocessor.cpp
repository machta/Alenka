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
	processorCacheBlockSizeCL = (blockSize + offset + padding)*file->getChannelCount();

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

	processorCacheBlockSizeGL = blockSize*montage->getNumberOfRows();

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

	for (int i = 0; i < dataFileCache.size(); ++i)
	{
		fprintf(stderr, "0x%p ", dataFileCache[i]);
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

	// Construct the processor cache.
	unsigned int processorCacheSize = PROGRAM_OPTIONS["processorQueues"].as<unsigned int>();

	processorCacheQueues.insert(processorCacheQueues.begin(), processorCacheSize, 0);
	processorCacheCLBuffers.insert(processorCacheCLBuffers.begin(), processorCacheSize, 0);
	processorCacheGLBuffers.insert(processorCacheGLBuffers.begin(), processorCacheSize, 0);
	processorCacheVertexArrays.insert(processorCacheVertexArrays.begin(), processorCacheSize, 0);

	fun()->glGenBuffers(processorCacheSize, processorCacheGLBuffers.data());
	fun()->glGenVertexArrays(processorCacheSize, processorCacheVertexArrays.data());

	for (unsigned int i = 0; i < processorCacheSize; ++i)
	{
		processorCacheQueues[i]  = clCreateCommandQueue(clContext->getCLContext(), clContext->getCLDevice(), 0, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

		processorCacheCLBuffers[i] = clCreateBuffer(clContext->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, processorCacheBlockSizeCL*sizeof(float), nullptr, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

		fun()->glBindVertexArray(processorCacheVertexArrays[i]);
		fun()->glBindBuffer(GL_ARRAY_BUFFER, processorCacheGLBuffers[i]);
		fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
		fun()->glEnableVertexAttribArray(0);
		fun()->glBufferData(GL_ARRAY_BUFFER, processorCacheBlockSizeGL*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	}

	fun()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	fun()->glBindVertexArray(0);

	processorCacheLogic = new PriorityCacheLogic(processorCacheSize);
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
	delete processorCacheLogic;

	err = clReleaseCommandQueue(gpuCacheQueue);
	checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");

	for (auto& e : processorCacheQueues)
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

	for (auto& e : processorCacheCLBuffers)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}

	fun()->glDeleteBuffers(processorCacheGLBuffers.size(), processorCacheGLBuffers.data());
	fun()->glDeleteVertexArrays(processorCacheVertexArrays.size(), processorCacheVertexArrays.data());
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	//processorCacheLogic->printInfo();

	unique_lock<mutex> processorCacheLock(processorCacheMutex);

	cl_int err;

	//prepareBlocks(indexSet, -1);

	while (1)
	{
		unsigned int processorCacheIndex;
		int blockIndex;

		// Fill.
		while (processorCacheLogic->fill(&processorCacheIndex, &blockIndex))
		{
			//processorCacheLogic->printInfo();

			processorCacheLock.unlock();

			unsigned int gpuCacheIndex;

			{
				unique_lock<mutex> gpuCacheLock(gpuCacheMutex);

				while (gpuCacheLogic->read(blockIndex, &gpuCacheIndex) == false)
				{
					fprintf(stderr, "processorCacheInCV(0x%p).wait()\n", &processorCacheInCV);
					processorCacheInCV.wait(gpuCacheLock);
				}
			}

			processorCacheLock.lock();

			filterProcessor->process(gpuCache[gpuCacheIndex], processorCacheCLBuffers[processorCacheIndex], processorCacheQueues[processorCacheIndex]);

			cl_mem buffer = clCreateFromGLBuffer(clContext->getCLContext(), CL_MEM_WRITE_ONLY, processorCacheGLBuffers[processorCacheIndex], &err);
			checkErrorCode(err, CL_SUCCESS, "clCreateFromGLBuffer()");

			montageProcessor->process(processorCacheCLBuffers[processorCacheIndex], buffer, processorCacheQueues[processorCacheIndex]);

			err = clReleaseMemObject(buffer);
			checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

			cl_event event;

			err = clEnqueueBarrierWithWaitList(processorCacheQueues[processorCacheIndex], 0, nullptr, &event);
			checkErrorCode(err, CL_SUCCESS, "clEnqueueBarrierWithWaitList()");

			processorCacheQueueCallbackData* data = new processorCacheQueueCallbackData {&processorCacheMutex, &processorCacheQueuesDone, blockIndex};

			err = clSetEventCallback(event, CL_COMPLETE, &processorCacheQueueCallback, data);
			checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

			err = clFlush(processorCacheQueues[processorCacheIndex]);
			checkErrorCode(err, CL_SUCCESS, "clFlush()");
		}

		processorCacheLock.unlock();
		processorCacheLock.lock();

		// Process the output from the callback function.
		while (processorCacheQueuesDone.empty() == false)
		{
			lock_guard<mutex> gpuCacheLock(gpuCacheMutex);

			gpuCacheLogic->release(processorCacheQueuesDone.back());
			processorCacheLogic->release(processorCacheQueuesDone.back());

			processorCacheQueuesDone.pop_back();
		}

		// Get.
		assert(indexSet.empty() == false);

		if (processorCacheLogic->readAny(indexSet, &processorCacheIndex, &blockIndex))
		{
			//processorCacheLogic->printInfo();

			auto fromTo = getBlockBoundaries(blockIndex);
			return SignalBlock(processorCacheVertexArrays[processorCacheIndex], processorCacheGLBuffers[processorCacheIndex],
							   blockIndex, montage->getNumberOfRows(), fromTo.first, fromTo.second);
		}
	}
}

#undef fun

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

				gpuCacheQueueCallbackData* data = new gpuCacheQueueCallbackData {&dataFileCacheMutex, &gpuCacheMutex, dataFileCacheLogic, gpuCacheLogic, &processorCacheInCV, blockIndex};

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

void SignalProcessor::processorCacheQueueCallback(cl_event event, cl_int event_command_exec_status, void* user_data)
{
	//fprintf(stderr, "processorCacheQueueCallback()\n");

	assert(event_command_exec_status == CL_COMPLETE);

	processorCacheQueueCallbackData* data = reinterpret_cast<processorCacheQueueCallbackData*>(user_data);

	lock_guard<mutex> lock(*get<0>(*data));

	get<1>(*data)->push_back(get<2>(*data));

	cl_int err = clReleaseEvent(event);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	delete data;
}
