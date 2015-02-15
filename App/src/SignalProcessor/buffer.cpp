#include "buffer.h"

#include <cassert>

using namespace std;

Buffer::Buffer(unsigned int numberOfBlocks, condition_variable cvs[2]) :
	inCV(cvs), outCV(cvs + 1),
	queue(queueComparator),
	bufferTable(numberOfBlocks)
{
	buffers.insert(buffers.begin(), numberOfBlocks, 0);
	fun()->glGenBuffers(numberOfBlocks, buffers.data());
}

Buffer::~Buffer()
{
	fun()->glDeleteBuffers(buffers.size(), buffers.data());
}

void Buffer::clearBuffer()
{

}

void Buffer::enqueue(const set<unsigned int>& index, int priority)
{
	lock_guard<mutex> lock(mtx);

	for (const auto& e : index)
	{
		queue.emplace(e, priority);
	}

	inCV->notify_all();
}

SignalBlock Buffer::fillBuffer(atomic<bool>* stop)
{
	unique_lock<mutex> lock(mtx);

	while (stop->load() == false)
	{
		if (queue.empty())
		{
			// The queue is empy -- wait until it gets populated.
			inCV->wait(lock);
			continue;
		}

		unsigned int blockIndex = queue.top().first;
		int priority = queue.top().second;

		if (blockBufferMap.count(blockIndex))
		{
			// This block is already buffered -- only update its priority.
			unsigned int bufferIndex = blockBufferMap[blockIndex];
			bufferTable.setPriority(bufferIndex, min(priority, bufferTable.getPriority(bufferIndex)));
			continue;
		}

		unsigned int lastBuffer = bufferTable.getLastOrder();

		if (bufferTable.getNotInUse(lastBuffer) == false || priority > bufferTable.getPriority(lastBuffer))
		{
			// There are no awailible buffers -- wait until some get released.
			inCV->wait(lock);
			continue;
		}

		assert(bufferTable.getNotInUse(lastBuffer) == true);
		assert(priority <= bufferTable.getPriority(lastBuffer));

		// Reserve the last buffer, update blockBufferMap and return the appropriate object.
		bufferTable.setPriority(lastBuffer, priority);
		bufferTable.setNotInUse(lastBuffer, false);

		blockBufferMap[blockIndex] = lastBuffer;

		return SignalBlock(buffers[lastBuffer], blockIndex);
	}
}

SignalBlock Buffer::readAnyBlock(const set<unsigned int>& index, atomic<bool>* stop)
{
	enqueue(index, -1);

	unique_lock<mutex> lock(mtx);

	while (stop->load() == false)
	{
		// Try to find if any block is already buffered.
		auto mapP = blockBufferMap.begin();
		auto setP = index.begin();
		int blockFound = -1, bufferIndex;
		while (mapP != blockBufferMap.end() && setP != index.end())
		{
			if (mapP->first == *setP && bufferTable.getNotInUse(mapP->first) == false)
			{
				blockFound = mapP->first;
				bufferIndex = mapP->second;
				break;
			}

			if (mapP->first < *setP)
			{
				mapP++;
			}
			else
			{
				setP++;
			}
		}

		if (blockFound == -1)
		{
			// No block found -- wait.
			outCV->wait(lock);
		}
		else
		{
			bufferTable.updateLastUsed(bufferIndex);
			bufferTable.setNotInUse(bufferIndex, false);

			return SignalBlock(buffers[bufferIndex], blockFound);
		}
	}
}

void Buffer::release(const SignalBlock& block)
{
	lock_guard<mutex> lock(mtx);

	releaseCommonPart(block);
}

void Buffer::release(const SignalBlock& block, int newPriority)
{
	lock_guard<mutex> lock(mtx);

	unsigned int bufferIndex = releaseCommonPart(block);

	bufferTable.setPriority(bufferIndex, newPriority);
}
