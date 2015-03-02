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

	err = clReleaseCommandQueue(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");

	for (auto& e : dataFileCache)
	{
		delete[] e;
	}

	for (auto& e : gpuCache)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}

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

	//gpuCacheLogic->printInfo();

	cl_int err;
	unsigned int gpuCacheIndex;
	int blockIndex;

	{
		unique_lock<mutex> lock(gpuCacheMutex);

		while (gpuCacheLogic->readAny(indexSet, &gpuCacheIndex, &blockIndex) == false)
		{
			//gpuCacheLogic->printInfo();

			thread t ([this, indexSet] () { prepareBlocks(indexSet, -1); });
			t.detach();

			//gpuCacheLogic->printInfo();

#if THREAD_DEBUG_OUTPUT
			fprintf(stderr, "processorInCV(0x%p).wait(%s)\n", &processorInCV, indexSetString(indexSet).c_str());
#endif
			processorInCV.wait(lock);

			//gpuCacheLogic->printInfo();
		}
	}

	//gpuCacheLogic->printInfo();

	filterProcessor->process(gpuCache[gpuCacheIndex], processorTmpBuffer, processorQueue);

	err = clFlush(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFlush()");

	{
		lock_guard<mutex> lock(gpuCacheMutex);

		gpuCacheLogic->release(blockIndex);

#if THREAD_DEBUG_OUTPUT
		fprintf(stderr, "gpuCacheOutCV(0x%p).notify_one(%d)\n", &gpuCacheOutCV, blockIndex);
#endif
		gpuCacheOutCV.notify_one();
	}

	fun()->glFinish();

	err = clEnqueueAcquireGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueAcquireGLObjects()");

	montageProcessor->process(processorTmpBuffer, processorOutputBuffer, processorQueue);

	err = clEnqueueReleaseGLObjects(processorQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
	checkErrorCode(err, CL_SUCCESS, "clEnqueueReleaseGLObjects()");

	err = clFinish(processorQueue);
	checkErrorCode(err, CL_SUCCESS, "clFinish()");

	auto fromTo = getBlockBoundaries(blockIndex);
	return SignalBlock(blockIndex, montage->getNumberOfRows(), fromTo.first, fromTo.second, processorVertexArray);
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

#if THREAD_DEBUG_OUTPUT
				fprintf(stderr, "gpuCacheInCV(0x%p).notify_one(%d)\n", &gpuCacheInCV, blockIndex);
#endif
				gpuCacheInCV.notify_one();
			}
			else
			{
#if THREAD_DEBUG_OUTPUT
				fprintf(stderr, "dataFileCacheOutCV(0x%p).wait(?)\n", &dataFileCacheOutCV);
#endif
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
						thread t ([this, blockIndex] () { prepareBlocks(set<int> {blockIndex}, -1); });
						t.detach();

#if THREAD_DEBUG_OUTPUT
						fprintf(stderr, "gpuCacheInCV(0x%p).wait(%d)\n", &gpuCacheInCV, blockIndex);
#endif
						gpuCacheInCV.wait(dataFileCacheLock);
					}
				}

				gpuCacheLock.lock();

				cl_int err;

				cl_event event;

				err = clEnqueueWriteBuffer(gpuCacheQueue, gpuCache[gpuCacheIndex], CL_FALSE, 0, dataFileGpuCacheBlockSize*sizeof(float), dataFileCache[dataFileCacheIndex], 0, nullptr, &event);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");

				gpuCacheQueueCallbackData* data = new gpuCacheQueueCallbackData {array<mutex*, 2> {&dataFileCacheMutex, &gpuCacheMutex}, array<PriorityCacheLogic*, 2> {dataFileCacheLogic, gpuCacheLogic}, array<condition_variable*, 3> {&dataFileCacheOutCV, &gpuCacheOutCV, &processorInCV}, blockIndex};

				err = clSetEventCallback(event, CL_COMPLETE, &gpuCacheQueueCallback, data);
				checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

				err = clFlush(gpuCacheQueue);
				checkErrorCode(err, CL_SUCCESS, "clFlush()");
			}
			else
			{
#if THREAD_DEBUG_OUTPUT
				fprintf(stderr, "gpuCacheOutCV(0x%p).wait(?)\n", &gpuCacheOutCV);
#endif
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

	int blockIndex = get<3>(*data);

	for (int i = 0; i < 2; ++i)
	{
		lock_guard<mutex> lock(*get<0>(*data)[i]);
		get<1>(*data)[i]->release(blockIndex);
	}

	for (const auto& e: get<2>(*data))
	{
#if THREAD_DEBUG_OUTPUT
		fprintf(stderr, "e(0x%p)->notify_one(%d)\n", e, blockIndex);
#endif
		e->notify_one();
	}

	cl_int err = clReleaseEvent(event);
	checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

	delete data;
}
