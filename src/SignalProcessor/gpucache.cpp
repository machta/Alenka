#include "gpucache.h"

#include "../DataModel/opendatafile.h"
#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filterprocessor.h>

#include <algorithm>
#include <cassert>

using namespace std;

namespace
{

/**
 * @brief Tries to find an element common to the map keys in a and the set elements in b.
 * @param index [out]
 * @param cacheIndex [out]
 * @return True if a common element was found.
 */
bool findCommon(const std::map<int, unsigned int>& a, const std::set<int>& b, int* index, unsigned int* cacheIndex)
{
	auto aI = a.begin();
	auto bI = b.begin();

	while (aI != a.end() && bI != b.end())
	{
		if (aI->first == *bI)
		{
			*index = aI->first;
			*cacheIndex = aI->second;
			return true;
		}

		if (aI->first < *bI)
			++aI;
		else
			++bI;
	}

	return false;
}

} // namespace

GPUCache::GPUCache(unsigned int blockSize, unsigned int offset, int delay, int64_t availableMemory, OpenDataFile* file, AlenkaSignal::OpenCLContext* context, AlenkaSignal::FilterProcessor<float>* filterProcessor)
	: blockSize(blockSize), offset(offset), delay(delay), file(file), filterProcessor(filterProcessor)
{
	cl_int err;

	unsigned int bytesPerBlock = (blockSize + offset + 4)*file->file->getChannelCount()*sizeof(float); // The +4 is pading for the filter processor
	capacity = availableMemory/bytesPerBlock;

	if (capacity == 0)
		throw runtime_error("Not enough memory for the gpu cache.");

	logToFile("Creating GPUCache with " << capacity << " blocks.");

	cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_1_2
	flags |= CL_MEM_HOST_WRITE_ONLY;
#endif
#endif

	for (unsigned int i = 0; i < capacity; ++i)
	{
		buffers.push_back(clCreateBuffer(context->getCLContext(), flags, bytesPerBlock, nullptr, &err));
		checkClErrorCode(err, "clCreateBuffer()");

		lastUsed.push_back(0);
		order.push_back(i);
	}

	tmpMemBuffer = clCreateBuffer(context->getCLContext(), flags, bytesPerBlock, nullptr, &err);
	checkClErrorCode(err, "clCreateBuffer()");

	commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
	checkClErrorCode(err, "clCreateCommandQueue()");

	loaderThread = thread(&GPUCache::loaderThreadFunction, this);

	tmpBuffer.resize((blockSize + offset)*file->file->getChannelCount());
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

	err = clFinish(commandQueue);
	checkClErrorCode(err, "clFinish()");

	err = clReleaseCommandQueue(commandQueue);
	checkClErrorCode(err, "clReleaseCommandQueue()");

	for (auto e : buffers)
	{
		err = clReleaseMemObject(e);
		checkClErrorCode(err, "clReleaseMemObject()");
	}

	err = clReleaseMemObject(tmpMemBuffer);
	checkClErrorCode(err, "clReleaseMemObject()");
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
			indexMap.erase(reverseIndexMap[cacheIndex]);

		indexMap[index] = cacheIndex;
		reverseIndexMap[cacheIndex] = index;

		assert(indexMap.size() == reverseIndexMap.size());

		//lock_guard<mutex> lock(loaderThreadMutex);
		//queue.emplace(index, cacheIndex, readyEvent, buffer);

		assert(queue.empty());
		loadOneBlock(index, cacheIndex, readyEvent, buffer);

		loaderThreadCV.notify_one();
	}

	// Update the order for next time this method is called.
	for (auto& e : lastUsed)
		++e;

	lastUsed[cacheIndex] = 0;

	sort(order.begin(), order.end(), [this] (unsigned int a, unsigned int b) { return lastUsed[a] < lastUsed[b]; });

	return index;
}

void GPUCache::loaderThreadFunction()
{
	return; // Temporarily disable this code.

	try
	{
		while (loaderThreadStop.load() == false)
		{
			unique_lock<mutex> lock(loaderThreadMutex);

			if (queue.empty() == false)
			{
				int index = get<0>(queue.front());
				unsigned int cacheIndex = get<1>(queue.front());
				cl_event readyEvent = get<2>(queue.front());
				cl_mem buffer = get<3>(queue.front());

				queue.pop();
				lock.unlock();

				loadOneBlock(index, cacheIndex, readyEvent, buffer);
			}
			else
			{
				loaderThreadCV.wait(lock);
			}
		}		
	}
	catch (exception& e)
	{
		logToFileAndConsole("Exception caught: " << e.what());
		abort();
	}
}

void GPUCache::enqueuCopy(cl_mem source, cl_mem destination, cl_event readyEvent)
{
	if (destination)
	{
		cl_int err;
		cl_event event;

		size_t origin[] = {0, 0, 0};
		size_t rowLen = (blockSize + offset)*sizeof(float);
		size_t region[] = {rowLen, file->file->getChannelCount(), 1};

		err = clEnqueueCopyBufferRect(commandQueue, source, destination, origin, origin, region, rowLen + 4*sizeof(float), 0, 0, 0, 0, nullptr, &event);
		checkClErrorCode(err, "clEnqueueCopyBufferRect()");

		//err = clEnqueueCopyBuffer(commandQueue, source, destination, 0, 0, (blockSize + offset)*file->file->getChannelCount()*sizeof(float), 0, nullptr, &event);
		//checkClErrorCode(err, "clEnqueueCopyBuffer()");

		err = clSetEventCallback(event, CL_COMPLETE, &signalEventCallback, readyEvent);
		checkClErrorCode(err, "clSetEventCallback()");

		err = clFlush(commandQueue);
		checkClErrorCode(err, "clFlush()");
	}
}

// AMD_BUG specific parts work around a bug in clEnqueueWriteBufferRect() when using some versions of AMD drivers.
// As a result of this bug, clEnqueueWriteBufferRect() copies only a part of the data to the buffer.
// The bug is reported here:
// http://devgurus.amd.com/thread/169828
// http://devgurus.amd.com/thread/160312

void GPUCache::loadOneBlock(int index, unsigned int cacheIndex, cl_event readyEvent, cl_mem buffer)
{
	logToFile("Loading block " << index << ".");

	cl_int err;
	auto fromTo = blockIndexToSampleRange(index, blockSize);

	if (tmpBufferEvent)
	{
		err = clWaitForEvents(1, &tmpBufferEvent);
		checkClErrorCode(err, "clWaitForEvents()");

		err = clReleaseEvent(tmpBufferEvent);
		checkClErrorCode(err, "clReleaseEvent()");
	}

	file->file->readSignal(tmpBuffer.data(), fromTo.first - offset + delay, fromTo.second + delay);

	printBuffer("after_readData.txt", tmpBuffer.data(), tmpBuffer.size());

#ifdef AMD_BUG
	err = clEnqueueWriteBuffer(commandQueue, tmpMemBuffer, CL_TRUE, 0, (blockSize + offset)*file->file->getChannelCount()*sizeof(float), tmpBuffer.data(), 0, nullptr, &tmpBufferEvent);
	checkClErrorCode(err, "clEnqueueWriteBuffer()");
#else
	size_t origin[] = {0, 0, 0};
	size_t rowLen = (blockSize + offset)*sizeof(float);
	size_t region[] = {rowLen, file->file->getChannelCount(), 1};

	err = clEnqueueWriteBufferRect(commandQueue, tmpMemBuffer/*buffers[cacheIndex]*/, /*CL_FALSE*/CL_TRUE, origin, origin, region, rowLen + 4*sizeof(float), 0, 0, 0, tmpBuffer.data(), 0, nullptr, &tmpBufferEvent);
	checkClErrorCode(err, "clEnqueueWriteBufferRect()");
#endif

	printBuffer("after_writeBuffer.txt", tmpMemBuffer, commandQueue);

	if (filterProcessor)
	{
		//clFinish(commandQueue);
		filterProcessor->process(tmpMemBuffer, buffers[cacheIndex], commandQueue);
		//clFinish(commandQueue);

		printBuffer("after_filter.txt", buffers[cacheIndex], commandQueue);
	}

	enqueuCopy(buffers[cacheIndex], buffer, readyEvent);
}

void CL_CALLBACK GPUCache::signalEventCallback(cl_event callbackEvent, cl_int status, void* data)
{
	try
	{
		assert(status == CL_COMPLETE);
		(void)status;

		cl_event event = reinterpret_cast<cl_event>(data);

		cl_int err;

		err = clSetUserEventStatus(event, CL_COMPLETE);
		checkClErrorCode(err, "clSetUserEventStatus()");

		err = clReleaseEvent(callbackEvent);
		checkClErrorCode(err, "clReleaseEvent()");
	}
	catch (exception& e)
	{
		logToFileAndConsole("Exception caught: " << e.what());
		abort();
	}
}
