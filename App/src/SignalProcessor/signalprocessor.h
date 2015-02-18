#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../openglinterface.h"

#include "signalblock.h"
#include "../DataFile/datafile.h"
#include "../options.h"
#include "prioritycachelogic.h"

#include <QOffscreenSurface>

#include <cinttypes>
#include <set>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, unsigned int memory = PROGRAM_OPTIONS->get("memoryBuffersSize").as<unsigned int>(), double bufferRatio = 1);
	~SignalProcessor();

	int64_t getBlockSize() const
	{
		return PROGRAM_OPTIONS->get("blockSize").as<unsigned int>();
	}

	// ..

	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void release(const SignalBlock& block)
	{
		std::lock_guard<std::mutex> lock(dataFileCacheMutex);
		dataFileCacheLogic->release(block.getIndex());
		dataFileCacheInCV.notify_one();
	}
	void release(const SignalBlock& block, int newPriority)
	{
		std::lock_guard<std::mutex> lock(dataFileCacheMutex);
		dataFileCacheLogic->release(block.getIndex(), newPriority);
		dataFileCacheInCV.notify_one();
	}
	void prepareBlocks(const std::set<int>& indexSet, int priority)
	{
		std::lock_guard<std::mutex> lock(dataFileCacheMutex);
		dataFileCacheLogic->enqueue(indexSet, priority);
		dataFileCacheInCV.notify_all();
	}

private:
	DataFile* dataFile;
	std::atomic<bool> threadsStop {false};
	GLuint vertexArray;
	GLuint buffer;

	std::mutex dataFileCacheMutex;
	std::condition_variable dataFileCacheInCV;
	std::condition_variable dataFileCacheOutCV;
	std::vector<float*> dataFileCache;
	PriorityCacheLogic* dataFileCacheLogic;
	std::thread dataFileCacherFillerThread;

	void dataFileCacheFiller(std::atomic<bool>* stop);
};

#endif // SIGNALPROCESSOR_H
