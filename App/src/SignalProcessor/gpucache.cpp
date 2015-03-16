#include "gpucache.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

GPUCache::GPUCache(unsigned int blockSize, int offset, int delay, unsigned int capacity, DataFile* file, OpenCLContext* context, FilterProcessor* filterProcessor)
	: blockSize(blockSize), offset(offset), delay(delay), capacity(capacity), file(file), filterProcessor(filterProcessor)
{
	cl_int err;

	for (unsigned int i = 0; i < capacity; ++i)
	{
		cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_VERSION_1_2
		flags |= CL_MEM_HOST_WRITE_ONLY;
#endif
#endif

		buffers.push_back(clCreateBuffer(context->getCLContext(), flags, (blockSize + offset + 4)*file->getChannelCount()*sizeof(float), nullptr, &err));
		checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

		lastUsed.push_back(0);
		order.push_back(i);
	}

	commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

	loaderThread = thread(&GPUCache::loaderThreadFunction, this);
}

GPUCache::~GPUCache()
{
	// Join the loader thread.
	loaderThreadStop.store(true);

	thread t([this] () { loaderThread.join(); });

	loaderThreadCV.notify_all();

	t.join();

	// Delete resources.
	cl_int err;

	for (auto& e : buffers)
	{
		err = clReleaseMemObject(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");
	}
}

int GPUCache::getAny(const set<int>& indexSet, cl_mem buffer, cl_event readyEvent)
{
	int index;
	unsigned int cacheIndex;

	if (findCommon(indexMap, indexSet, &index, &cacheIndex))
	{
		enqueuCopy(buffers[cacheIndex], buffer, readyEvent);
	}
	else
	{
		index = *indexSet.begin();
		cacheIndex = order.back();

		if (reverseIndexMap.count(cacheIndex))
		{
			indexMap.erase(reverseIndexMap[cacheIndex]);
		}

		indexMap[index] = cacheIndex;
		reverseIndexMap[cacheIndex] = index;

		assert(indexMap.size() == reverseIndexMap.size());

		lock_guard<mutex> lock(loaderThreadMutex);

		queue.emplace(index, cacheIndex, readyEvent, buffer);

		loaderThreadCV.notify_one();
	}

	// Update the order for next time this method is called.
	for (auto& e : lastUsed)
	{
		++e;
	}

	lastUsed[cacheIndex] = 0;

	sort(order.begin(), order.end(), [this] (unsigned int a, unsigned int b) { return lastUsed[a] < lastUsed[b]; });

	return index;
}

// AMD_BUG specific parts work around a bug in clEnqueueWriteBufferRect() when using some versions of AMD drivers.
// As a result of this bug, clEnqueueWriteBufferRect() copies only a part of the data to the buffer.
// The bug is reported here:
// http://devgurus.amd.com/thread/169828
// http://devgurus.amd.com/thread/160312

void GPUCache::loaderThreadFunction()
{
	cl_int err;

	try
	{
		int size = (blockSize + offset)*file->getChannelCount();
#ifdef AMD_BUG
		size += 4*file->getChannelCount();
#endif

		vector<float> tmpBuffer(size);

		while (loaderThreadStop.load() == false)
		{
			unique_lock<mutex> lock(loaderThreadMutex);

			if (queue.empty() == false)
			{
				int index = get<0>(queue.front());
				unsigned int cacheIndex = get<1>(queue.front());
				cl_event readyEvent = get<2>(queue.front());
				cl_mem buffer = get<3>(queue.front());

				logToFile("Loading block " << index << ".");

				queue.pop();

				lock.unlock();

				auto fromTo = file->getBlockBoundaries(index, blockSize);
#ifdef AMD_BUG
				fromTo.second += 4;
#endif

				file->readData(&tmpBuffer, fromTo.first - offset + delay, fromTo.second + delay);

				printBuffer("after_readData.txt", tmpBuffer.data(), tmpBuffer.size());

				size_t origin[] = {0, 0, 0};
				size_t rowLen = (blockSize + offset)*sizeof(float);
				size_t region[] = {rowLen, file->getChannelCount(), 1};

#ifdef AMD_BUG
				err = clEnqueueWriteBuffer(commandQueue, buffers[cacheIndex], CL_TRUE, 0, (blockSize + offset + 4)*file->getChannelCount()*sizeof(float), tmpBuffer.data(), 0, nullptr, nullptr);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");
#else
				err = clEnqueueWriteBufferRect(commandQueue, buffers[cacheIndex], CL_TRUE, origin, origin, region, rowLen + 4*sizeof(float), 0, 0, 0, tmpBuffer.data(), 0, nullptr, nullptr);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBufferRect()");
#endif

				printBuffer("after_writeBuffer.txt", buffers[cacheIndex], commandQueue);

				if (filterProcessor != nullptr)
				{
					filterProcessor->process(buffers[cacheIndex], commandQueue);

					printBuffer("after_filter.txt", buffers[cacheIndex], commandQueue);
				}

				enqueuCopy(buffers[cacheIndex], buffer, readyEvent);
			}
			else
			{
				loaderThreadCV.wait(lock);
			}
		}

		err = clReleaseCommandQueue(commandQueue);
		checkErrorCode(err, CL_SUCCESS, "clReleaseCommandQueue()");
	}
	catch (exception& e)
	{
		logToBoth("Exception caught: " << e.what());
		abort();
	}
}

void GPUCache::enqueuCopy(cl_mem source, cl_mem destination, cl_event readyEvent)
{
	if (destination != nullptr)
	{
		cl_int err;
		cl_event event;

		err = clEnqueueCopyBuffer(commandQueue, source, destination, 0, 0, (blockSize + offset + 4)*file->getChannelCount()*sizeof(float), 0, nullptr, &event);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueCopyBuffer()");

		err = clSetEventCallback(event, CL_COMPLETE, &signalEventCallback, readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

		err = clFlush(commandQueue);
		checkErrorCode(err, CL_SUCCESS, "clFlush()");
	}
}

void GPUCache::signalEventCallback(cl_event callbackEvent, cl_int status, void* data)
{
	try
	{
		assert(status == CL_COMPLETE);
		(void)status;

		cl_event event = reinterpret_cast<cl_event>(data);

		cl_int err;

		err = clSetUserEventStatus(event, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");

		err = clReleaseEvent(callbackEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	catch (exception& e)
	{
		logToBoth("Exception caught: " << e.what());
		abort();
	}
}
