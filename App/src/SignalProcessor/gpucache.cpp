#include "gpucache.h"

#include <algorithm>
#include <cassert>

using namespace std;

GPUCache::GPUCache(unsigned int blockSize, int offset, int delay, unsigned int capacity, DataFile* file, OpenCLContext* context)
	: blockSize(blockSize), offset(offset), delay(delay), capacity(capacity), file(file), context(context)
{
	cl_int err;

	for (unsigned int i = 0; i < capacity; ++i)
	{
		buffers.push_back(clCreateBuffer(context->getCLContext(), CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, (blockSize + offset)*file->getChannelCount()*sizeof(float), nullptr, &err));
		checkErrorCode(err, CL_SUCCESS, "clCreateBuffer()");

		doneEvents.push_back(clCreateUserEvent(context->getCLContext(), &err));
		checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

		cerr << "Create init done event " << doneEvents.back() << endl;

		err = clSetUserEventStatus(doneEvents.back(), CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");

		cerr << "Signal init done event " << doneEvents.back() << endl;

		lastUsed.push_back(0);
		order.push_back(i);
	}

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

	for (auto& e : doneEvents)
	{
		err = clReleaseEvent(e);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
}

int GPUCache::getAny(const set<int>& indexSet, cl_mem* buffer, cl_event* readyEvent, cl_event* doneEvent)
{
	cl_int err;
	int index;
	unsigned int cacheIndex;

	*readyEvent = clCreateUserEvent(context->getCLContext(), &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

	*doneEvent = clCreateUserEvent(context->getCLContext(), &err);
	checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

	cerr << "Create ready event " << *readyEvent << endl;
	cerr << "Create done event " << *doneEvent << endl;

	if (findCommon(indexMap, indexSet, &index, &cacheIndex))
	{
		lock_guard<mutex> lock(loaderThreadMutex);

		cl_int status;
		assert((err = clGetEventInfo(doneEvents[cacheIndex], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &status, nullptr), status == CL_COMPLETE && err == CL_SUCCESS));

		cerr << "Release done event after find " << doneEvents[cacheIndex] << endl;

		err = clReleaseEvent(doneEvents[cacheIndex]);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

		doneEvents[cacheIndex] = *doneEvent;

		cerr << "Signal ready event after find " << *readyEvent << endl;

		err = clSetUserEventStatus(*readyEvent, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");
	}
	else
	{
		index = *indexSet.begin();
		cacheIndex = order.back();

		reverseIndexMap.erase(indexMap[index]);

		indexMap[index] = cacheIndex;
		reverseIndexMap[cacheIndex] = index;

		if (indexMap.size() != reverseIndexMap.size())
		{
			int dummy = 5;
			(void)dummy;
		}

		assert(indexMap.size() == reverseIndexMap.size());

		lock_guard<mutex> lock(loaderThreadMutex);

		queue.emplace(index, cacheIndex, *readyEvent, *doneEvent);

		loaderThreadCV.notify_one();
	}

	// Update the order for next time this method is called.
	for (auto& e : lastUsed)
	{
		++e;
	}

	lastUsed[cacheIndex] = 0;

	sort(order.begin(), order.end(), [this] (unsigned int a, unsigned int b) { return lastUsed[a] < lastUsed[b]; });

	*buffer = buffers[cacheIndex];
	return index;
}

void GPUCache::loaderThreadFunction()
{
	cl_int err;

	try
	{
		vector<float> tmpBuffer((blockSize + offset)*file->getChannelCount());

		cl_command_queue commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0/*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/, &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateCommandQueue()");

		while (loaderThreadStop.load() == false)
		{
			unique_lock<mutex> lock(loaderThreadMutex);

			if (queue.empty() == false)
			{
				int index = get<0>(queue.front());
				unsigned int cacheIndex = get<1>(queue.front());
				cl_event readyEvent = get<2>(queue.front());
				cl_event doneEvent = get<3>(queue.front());

				cerr << "Queue top: " << index << ", " << cacheIndex << ", " << readyEvent << ", " << doneEvent << endl;

				queue.pop();

				//lock.unlock();

				auto fromTo = file->getBlockBoundaries(index, blockSize);

				file->readData(&tmpBuffer, fromTo.first - offset + delay, fromTo.second + delay);

				//lock.lock();

				cl_event event;
				cl_int err = clEnqueueWriteBuffer(commandQueue, buffers[cacheIndex], CL_FALSE, 0, (blockSize + offset)*file->getChannelCount()*sizeof(float), tmpBuffer.data(), 1, &doneEvents[cacheIndex], &event);
				checkErrorCode(err, CL_SUCCESS, "clEnqueueWriteBuffer()");

				cerr << "Release done event " << doneEvents[cacheIndex] << endl;

				err = clReleaseEvent(doneEvents[cacheIndex]);
				checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

				doneEvents[cacheIndex] = doneEvent;

				err = clSetEventCallback(event, CL_COMPLETE, &signalEventCallbackReady, &readyEvent);
				checkErrorCode(err, CL_SUCCESS, "clSetEventCallback()");

				err = clFlush(commandQueue);
				checkErrorCode(err, CL_SUCCESS, "clFlush()");
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
