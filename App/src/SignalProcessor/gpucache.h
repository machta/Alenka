#ifndef GPUCACHE_H
#define GPUCACHE_H

#include "../DataFile/datafile.h"
#include "../error.h"
#include "../openclcontext.h"

#include <CL/cl_gl.h>

#include <map>
#include <vector>
#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <exception>
#include <tuple>
#include <iostream>

class GPUCache
{
public:
	GPUCache(unsigned int blockSize, int offset, int delay, unsigned int capacity, DataFile* file, OpenCLContext* context);
	~GPUCache();

	int getAny(const std::set<int> &indexSet, cl_mem* buffer, cl_event* readyEvent, cl_event* doneEvent);
	void clear()
	{
		indexMap.clear();
		reverseIndexMap.clear();
	}
	unsigned int getCapacity() const
	{
		return capacity;
	}
	static void signalEventCallbackReady(cl_event callbackEvent, cl_int status, void* data)
	{
		assert(status == CL_COMPLETE);

		cl_event event = *reinterpret_cast<cl_event*>(data);

		std::cerr << "Signal event ready " << event << std::endl;

		cl_int err;

		err = clSetUserEventStatus(event, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");

		err = clReleaseEvent(callbackEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	static void signalEventCallbackDone(cl_event callbackEvent, cl_int status, void* data)
	{
		assert(status == CL_COMPLETE);

		cl_event event = *reinterpret_cast<cl_event*>(data);

		std::cerr << "Signal event done " << event << std::endl;

		cl_int err;

		err = clSetUserEventStatus(event, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");

		err = clReleaseEvent(callbackEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}

private:
	unsigned int blockSize;
	int offset;
	int delay;
	unsigned int capacity;
	DataFile* file;
	OpenCLContext* context;
	std::vector<cl_mem> buffers;
	std::vector<cl_event> doneEvents;
	std::vector<unsigned int> lastUsed;
	std::vector<unsigned int> order;
	std::map<int, unsigned int> indexMap;
	std::map<unsigned int, int> reverseIndexMap;
	std::mutex loaderThreadMutex;
	std::queue<std::tuple<int, unsigned int, cl_event, cl_event>> queue;
	std::thread loaderThread;
	std::condition_variable loaderThreadCV;
	std::atomic<bool> loaderThreadStop {false};

	void loaderThreadFunction();

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
			{
				++aI;
			}
			else
			{
				++bI;
			}
		}

		return false;
	}
};

#endif // GPUCACHE_H
