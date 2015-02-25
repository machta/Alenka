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

using cacheCallbackData = std::tuple<std::recursive_mutex*, std::recursive_mutex*, PriorityCacheLogic*, PriorityCacheLogic*, std::condition_variable_any*, int>;

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
	void release(const SignalBlock& block)
	{
		std::lock_guard<std::recursive_mutex> lock(processorCacheMutex);
		processorCacheLogic->release(block.getIndex());
	}
	void release(const SignalBlock& block, int newPriority)
	{
		std::lock_guard<std::recursive_mutex> lock(processorCacheMutex);
		processorCacheLogic->release(block.getIndex(), newPriority);
	}
	void prepareBlocks(const std::set<int>& indexSet, int priority)
	{
		{
			std::lock_guard<std::recursive_mutex> lock(dataFileCacheMutex);
			dataFileCacheLogic->enqueue(indexSet, priority);
		}

		{
			std::lock_guard<std::recursive_mutex> lock(gpuCacheMutex);
			gpuCacheLogic->enqueue(indexSet, priority);
		}

		{
			std::lock_guard<std::recursive_mutex> lock(processorCacheMutex);
			processorCacheLogic->enqueue(indexSet, priority);
		}

		fprintf(stderr, "inCV(0x%p).notify_all()\n", &inCV);
		inCV.notify_all();
		fprintf(stderr, "dataFileGpuCV(0x%p).notify_all()\n", &dataFileGpuCV);
		dataFileGpuCV.notify_all();
		fprintf(stderr, "gpuProcessorCV(0x%p).notify_all()\n", &gpuProcessorCV);
		gpuProcessorCV.notify_all();
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
	unsigned int processorCacheBlockSizeCL;
	unsigned int processorCacheBlockSizeGL;

	std::condition_variable_any inCV;
	std::condition_variable_any dataFileGpuCV;
	std::condition_variable_any gpuProcessorCV;
	std::condition_variable_any outCV;

	std::recursive_mutex dataFileCacheMutex;
	std::vector<float*> dataFileCache;
	PriorityCacheLogic* dataFileCacheLogic;
	std::thread dataFileCacheFillerThread;

	std::recursive_mutex gpuCacheMutex;
	std::vector<cl_mem> gpuCache;
	PriorityCacheLogic* gpuCacheLogic;
	std::thread gpuCacheFillerThread;
	cl_command_queue gpuCacheQueue;

	std::recursive_mutex processorCacheMutex;
	PriorityCacheLogic* processorCacheLogic;
	std::vector<cl_command_queue> processorCacheQueues;
	std::vector<cl_mem> processorCacheCLBuffers;
	std::vector<GLuint> processorCacheGLBuffers;
	std::vector<GLuint> processorCacheVertexArrays;

	void dataFileCacheFiller(std::atomic<bool>* stop);
	void gpuCacheFiller(std::atomic<bool>* stop);
	static void cacheCallback (cl_event event, cl_int event_command_exec_status, void* user_data);
	std::pair<std::int64_t, std::int64_t> getBlockBoundaries(int index)
	{
		int64_t from = index*getBlockSize(),
				to = from + getBlockSize() - 1;

		return std::pair<std::int64_t, std::int64_t>(from, to);
	}
};

#endif // SIGNALPROCESSOR_H
