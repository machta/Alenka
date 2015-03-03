#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#define THREAD_DEBUG_OUTPUT 0

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
#include <array>
#include <sstream>

using gpuCacheQueueCallbackData = std::tuple<std::array<std::mutex*, 2>, std::array<PriorityCacheLogic*, 2>, std::array<std::condition_variable*, 3>, int>;

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

#if THREAD_DEBUG_OUTPUT
		fprintf(stderr, "dataFileCacheOutCV(0x%p).notify_all()\n", &dataFileCacheOutCV);
#endif
		dataFileCacheOutCV.notify_all();

#if THREAD_DEBUG_OUTPUT
		fprintf(stderr, "gpuCacheOutCV(0x%p).notify_all()\n", &gpuCacheOutCV);
#endif
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
	std::condition_variable dataFileCacheOutCV;
	std::vector<std::vector<float>> dataFileCache;
	PriorityCacheLogic* dataFileCacheLogic;
	std::thread dataFileCacheFillerThread;

	std::mutex gpuCacheMutex;
	std::condition_variable gpuCacheInCV;
	std::condition_variable gpuCacheOutCV;
	std::vector<cl_mem> gpuCache;
	PriorityCacheLogic* gpuCacheLogic;
	std::thread gpuCacheFillerThread;
	cl_command_queue gpuCacheQueue;

	std::condition_variable processorInCV;
	cl_command_queue processorQueue;
	cl_mem processorTmpBuffer;
	cl_mem processorOutputBuffer;
	GLuint processorVertexArray;

	void dataFileCacheFiller(std::atomic<bool>* stop);
	void gpuCacheFiller(std::atomic<bool>* stop);
	static void gpuCacheQueueCallback(cl_event event, cl_int event_command_exec_status, void* user_data);
	std::pair<std::int64_t, std::int64_t> getBlockBoundaries(int index)
	{
		int64_t from = index*getBlockSize(),
				to = from + getBlockSize() - 1;

		return std::pair<std::int64_t, std::int64_t>(from, to);
	}	
	std::string indexSetString(const std::set<int>& indexSet)
	{
		std::stringstream ss;

		for (const auto& e : indexSet)
		{
			if (e != *indexSet.begin())
			{
				ss << ", ";
			}
			ss << e;
		}

		return ss.str();
	}
};

#endif // SIGNALPROCESSOR_H
