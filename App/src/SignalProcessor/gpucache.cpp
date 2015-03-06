#include "gpucache.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

GPUCache::GPUCache(unsigned int blockSize, int offset, int delay, unsigned int capacity, DataFile* file, OpenCLContext* context)
	: blockSize(blockSize), offset(offset), delay(delay), capacity(capacity), file(file)
{
	cl_int err;

	for (unsigned int i = 0; i < capacity; ++i)
	{
		cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
		flags |= CL_MEM_HOST_WRITE_ONLY;
#endif

		buffers.push_back(clCreateBuffer(context->getCLContext(), flags, (blockSize + offset)*file->getChannelCount()*sizeof(float), nullptr, &err));
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
		//lock_guard<mutex> lock(loaderThreadMutex);

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

void GPUCache::loaderThreadFunction()
{
	cl_int err;

	try
	{
		vector<float> tmpBuffer((blockSize + offset)*file->getChannelCount());

		while (loaderThreadStop.load() == false)
		{
			unique_lock<mutex> lock(loaderThreadMutex);

			if (queue.empty() == false)
			{
				int index = get<0>(queue.front());
				unsigned int cacheIndex = get<1>(queue.front());
				cl_event readyEvent = get<2>(queue.front());
				cl_mem buffer = get<3>(queue.front());

				cerr << "Queue top: " << index << ", " << cacheIndex << ", " << readyEvent  << "(" << &readyEvent << ")" << ", " << buffer << endl;

				queue.pop();

				lock.unlock();

				auto fromTo = file->getBlockBoundaries(index, blockSize);
				file->readData(&tmpBuffer, fromTo.first - offset + delay, fromTo.second + delay);

				printBuffer("after_readData.txt", tmpBuffer.data(), tmpBuffer.size());

				cl_int err = clEnqueueWriteBuffer(commandQueue, buffers[cacheIndex], CL_FALSE, 0, (blockSize + offset)*file->getChannelCount()*sizeof(float), tmpBuffer.data(), 0, nullptr, nullptr);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");

				printBuffer("after_writeBuffer.txt", buffers[cacheIndex], commandQueue);

				enqueuCopy(buffers[cacheIndex], buffer, readyEvent);
			}
			else
			{
				loaderThreadCV.wait(lock);
			}
		}

		err = clReleaseCommandQueue(commandQueue);
		checkErrorCode(err, CL_SUCCESS, "clFlush()");
	}
	catch (exception& e)
	{
		cerr << "Exception caught in loaderThreadFunction(): " << e.what() << endl;
		abort();
	}
}

void GPUCache::enqueuCopy(cl_mem source, cl_mem destination, cl_event readyEvent)
{
	if (destination != nullptr)
	{
		cl_int err;
		cl_event event;
		size_t origin[] = {0, 0, 0};
		size_t rowLen = (blockSize + offset)*sizeof(float);
		size_t region[] = {rowLen, file->getChannelCount(), 1};

		err = clEnqueueCopyBufferRect(commandQueue, source, destination, origin, origin, region, rowLen, 0, rowLen + 4*sizeof(float), 0, 0, nullptr, &event);
		checkErrorCode(err, CL_SUCCESS, "clEnqueueCopyBufferRect()");

		cerr << "Setting callback for event " << readyEvent << "(" << &readyEvent << ")" << endl;

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

		cl_event event = reinterpret_cast<cl_event>(data);

		cl_int err;

		cerr << "Signal event " << event << "(" << data << ")" << endl;

		err = clSetUserEventStatus(event, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");

		err = clReleaseEvent(callbackEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	catch (exception& e)
	{
		cerr << "Exception caught in signalEventCallback(): " << e.what() << endl;
		abort();
	}
}
