#ifndef GPUCACHE_H
#define GPUCACHE_H

#include "../DataFile/datafile.h"
#include "../error.h"
#include "../openclcontext.h"
#include "filterprocessor.h"

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

class GPUCache
{
public:
	GPUCache(unsigned int blockSize, int offset, int delay, int64_t availableMemory, DataFile* file, OpenCLContext* context, FilterProcessor* filterProcessor);
	~GPUCache();

	int getAny(const std::set<int> &indexSet, cl_mem buffer, cl_event readyEvent);
	unsigned int getCapacity() const
	{
		return capacity;
	}
	void clear()
	{
		indexMap.clear();
		reverseIndexMap.clear();
	}

private:
	unsigned int blockSize;
	int offset;
	int delay;
	unsigned int capacity;
	DataFile* file;
	FilterProcessor* filterProcessor;
	cl_command_queue commandQueue;
	std::vector<cl_mem> buffers;
	std::vector<unsigned int> lastUsed;
	std::vector<unsigned int> order;
	std::map<int, unsigned int> indexMap;
	std::map<unsigned int, int> reverseIndexMap;
	std::mutex loaderThreadMutex;
	std::queue<std::tuple<int, unsigned int, cl_event, cl_mem>> queue;
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
	void enqueuCopy(cl_mem source, cl_mem destination, cl_event readyEvent);
	static void CL_CALLBACK signalEventCallback(cl_event callbackEvent, cl_int status, void* data);
};

#endif // GPUCACHE_H
