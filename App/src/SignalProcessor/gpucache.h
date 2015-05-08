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

/**
 * @brief This class a LRU cache of data blocks on the GPU.
 *
 * This is a least-recently-used cache.
 *
 * It loads data from disk and copies it to buffer objects on the GPU.
 * The loading is done in a worker thread.
 *
 * The buffers are created in the constructor which means that the cache cannot
 * be resized.
 */
class GPUCache
{
public:
	/**
	 * @brief GPUCache
	 * @param offset Add this many samples to the size of the blockSize.
	 * @param delay Delay of the filter.
	 * @param availableMemory The maximum bytes available for the cache.
	 * @param filterProcessor If nullptr, the cached blocks are not filtered.
	 */
	GPUCache(unsigned int blockSize, unsigned int offset, int delay, int64_t availableMemory, DataFile* file, OpenCLContext* context, FilterProcessor* filterProcessor);
	~GPUCache();

	/**
	 * @brief Copy data of any block from indexSet to the buffer.
	 * @param indexSet Requested block indexes.
	 * @param buffer [out] A buffer to copy the data to.
	 * @param readyEvent This event is signaled once the data in the buffer is ready.
	 * @return The index of the block.
	 *
	 * Returns immediately. Use readyEvent for synchronization.
	 */
	int getAny(const std::set<int>& indexSet, cl_mem buffer, cl_event readyEvent);

	/**
	 * @brief Returns the maximum number of blocks that can be stored in the cache at the same time.
	 */
	unsigned int getCapacity() const
	{
		return capacity;
	}

	/**
	 * @brief Clears all cached blocks from the cache.
	 */
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

	/**
	 * @brief The code executed by the loader thread.
	 *
	 * The signal data is read from the DataFile by this thread.
	 *
	 * The work for this thread is added to queue. This thread loads the requested
	 * blocks one by one until it runs out of work. Then it sleeps.
	 */
	void loaderThreadFunction();

	/**
	 * @brief Tries to find an element common to the map keys in a and the set elements in b.
	 * @param a
	 * @param b
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

	/**
	 * @brief Enqueues command for copying of buffers.
	 * @param readyEvent This event is signaled once the data in the destination is ready.
	 */
	void enqueuCopy(cl_mem source, cl_mem destination, cl_event readyEvent);

	/**
	 * @brief Enqueues command for copying of buffers.
	 * @param callbackEvent The source event.
	 * @param status The status of the callbackEvent at the time of execution of this callback.
	 * @param data The target event.
	 */
	static void CL_CALLBACK signalEventCallback(cl_event callbackEvent, cl_int status, void* data);
};

#endif // GPUCACHE_H
