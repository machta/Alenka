#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../openglinterface.h"

#include "signalblock.h"
#include "../DataFile/datafile.h"
#include "../options.h"
#include "prioritycachelogic.h"
#include "../openclcontext.h"
#include "filterprocessor.h"
#include "montageprocessor.h"

#include <QOffscreenSurface>
#include <CL/cl_gl.h>

#include <cinttypes>
#include <set>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <tuple>

using gpuCacheQueueCallbackData = std::tuple<std::mutex*, std::mutex*, PriorityCacheLogic*, PriorityCacheLogic*, std::condition_variable_any*, int>;
using processorQueueCallbackData = std::tuple<std::mutex*, PriorityCacheLogic*, int>;

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, unsigned int memory = PROGRAM_OPTIONS["memoryBuffersSize"].as<unsigned int>(), double bufferRatio = 1);
	~SignalProcessor();

	int64_t getBlockSize() const
	{
		return blockSize;
	}

	// ..

	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void prepareBlocks(const std::set<int>& indexSet, int priority)
	{
		{
			std::lock_guard<std::mutex> lock(dataFileCacheMutex);
			dataFileCacheLogic->enqueue(indexSet, priority);
		}

		{
			std::lock_guard<std::mutex> lock(gpuCacheMutex);
			gpuCacheLogic->enqueue(indexSet, priority);
		}

		fprintf(stderr, "dataFileCacheOutCV(0x%p).notify_all()\n", &dataFileCacheOutCV);
		dataFileCacheOutCV.notify_all();
		fprintf(stderr, "gpuCacheOutCV(0x%p).notify_all()\n", &gpuCacheOutCV);
		gpuCacheOutCV.notify_all();
	}

private:
	DataFile* dataFile;
	std::atomic<bool> threadsStop {false};
	GLuint vertexArray;
	GLuint buffer;
	OpenCLContext* clContext;
	FilterProcessor* filterProcessor;
	Filter* filter;
	MontageProcessor* montageProcessor;
	Montage* montage;

	int M;
	int offset;
	int delay;
	int padding;

	unsigned int blockSize;
	unsigned int dataFileGpuCacheBlockSize;
	unsigned int processorTmpBlockSize;
	unsigned int processorOutputBlockSize;

	std::mutex dataFileCacheMutex;
	std::condition_variable_any dataFileCacheOutCV;
	std::vector<float*> dataFileCache;
	PriorityCacheLogic* dataFileCacheLogic;
	std::thread dataFileCacheFillerThread;

	std::mutex gpuCacheMutex;
	std::condition_variable_any gpuCacheInCV;
	std::condition_variable_any gpuCacheOutCV;
	std::vector<cl_mem> gpuCache;
	PriorityCacheLogic* gpuCacheLogic;
	std::thread gpuCacheFillerThread;
	cl_command_queue gpuCacheQueue;

	std::condition_variable_any processorInCV;
	unsigned int processorQueuesCount;
	unsigned int processorQueuesIndex = 0;
	std::vector<cl_command_queue> processorQueues;
	std::vector<cl_mem> processorTmpBuffers;
	std::vector<cl_mem> processorOutputBuffers;
	std::vector<GLuint> processorVertexArrays;

	void dataFileCacheFiller(std::atomic<bool>* stop);
	void gpuCacheFiller(std::atomic<bool>* stop);
	static void gpuCacheQueueCallback(cl_event event, cl_int event_command_exec_status, void* user_data);
	static void processorQueueCallback(cl_event event, cl_int event_command_exec_status, void* user_data);
	std::pair<std::int64_t, std::int64_t> getBlockBoundaries(int index)
	{
		int64_t from = index*getBlockSize(),
				to = from + getBlockSize() - 1;

		return std::pair<std::int64_t, std::int64_t>(from, to);
	}
};

#endif // SIGNALPROCESSOR_H
