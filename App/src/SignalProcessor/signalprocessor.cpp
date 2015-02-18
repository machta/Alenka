#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>

using namespace std;

#define fun() fun_shortcut()

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory, double /*bufferRatio*/) : dataFile(file)
{
	unsigned int dataFileCacheBlockSize = getBlockSize()*file->getChannelCount();

	unsigned int dataFileCacheBlockCount = memory/dataFileCacheBlockSize/sizeof(float);
	if (dataFileCacheBlockCount <= 0)
	{
		throw runtime_error("Not enough available memory for the dataFileCache");
	}

	dataFileCache.insert(dataFileCache.begin(), dataFileCacheBlockCount, nullptr);
	for (auto& e : dataFileCache)
	{
		e = new float[dataFileCacheBlockSize];
	}

	dataFileCacheLogic = new PriorityCacheLogic(dataFileCacheBlockCount);
	dataFileCacherFillerThread = thread(&SignalProcessor::dataFileCacheFiller, this, &threadsStop);

	// OpenGL stuff.
	fun()->glGenVertexArrays(1, &vertexArray);
	fun()->glBindVertexArray(vertexArray);

	fun()->glGenBuffers(1, &buffer);
	fun()->glBindBuffer(GL_ARRAY_BUFFER, buffer);

	fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	fun()->glEnableVertexAttribArray(0);

	if (!REALLOCATE_BUFFER)
	{
		fun()->glBufferData(GL_ARRAY_BUFFER, dataFileCacheBlockSize*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	}

	fun()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	fun()->glBindVertexArray(0);
}

SignalProcessor::~SignalProcessor()
{
	// Join dataFileCacherFillerThread.
	threadsStop.store(true);

	thread t([this] () { dataFileCacherFillerThread.join();	});

	dataFileCacheInCV.notify_all();
	//dataFileCacheOutCV.notify_all();

	t.join();

	// Release resources.
	delete dataFileCacheLogic;

	for (auto& e : dataFileCache)
	{
		delete[] e;
	}

	fun()->glDeleteBuffers(1, &buffer);
	fun()->glDeleteVertexArrays(1, &vertexArray);
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	unique_lock<mutex> lock(dataFileCacheMutex);

	while (1)
	{
		unsigned int cacheIndex;
		int blockIndex;

		bool blockFound = dataFileCacheLogic->readAny(indexSet, &cacheIndex, &blockIndex);

		if (blockFound)
		{
			fun()->glBindBuffer(GL_ARRAY_BUFFER, buffer);

			int64_t from = blockIndex*getBlockSize(),
					to = from + getBlockSize() - 1;

			size_t size = getBlockSize()*dataFile->getChannelCount()*sizeof(float);

			if (REALLOCATE_BUFFER)
			{
				fun()->glBufferData(GL_ARRAY_BUFFER, size, dataFileCache[cacheIndex], GL_STATIC_DRAW);
			}
			else
			{
				fun()->glBufferSubData(GL_ARRAY_BUFFER, 0, size, dataFileCache[cacheIndex]);
			}

			fun()->glBindBuffer(GL_ARRAY_BUFFER, 0);

			return SignalBlock(vertexArray, buffer, blockIndex, dataFile->getChannelCount(), from, to);
		}
		else
		{
			// The same as 'prepareBlocks(indexSet, -1);'
			dataFileCacheLogic->enqueue(indexSet, -1);
			dataFileCacheInCV.notify_all();

			dataFileCacheOutCV.wait(lock);
		}
	}
}

#undef fun

void SignalProcessor::dataFileCacheFiller(atomic<bool>* stop)
{
	unique_lock<mutex> lock(dataFileCacheMutex);

	while (stop->load() == false)
	{
		unsigned int cacheIndex;
		int blockIndex;

		bool notEmpty = dataFileCacheLogic->fill(&cacheIndex, &blockIndex);

		if (notEmpty)
		{
			int64_t from = blockIndex*getBlockSize(),
					to = from + getBlockSize() - 1;

			dataFile->readData(dataFileCache[cacheIndex], from, to);

			dataFileCacheLogic->release(blockIndex);

			dataFileCacheOutCV.notify_one();
		}
		else
		{
			dataFileCacheInCV.wait(lock);
		}
	}
}

