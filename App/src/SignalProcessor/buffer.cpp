#include "buffer.h"

#include <cassert>

using namespace std;

#define fun() fun_shortcut()

Buffer::Buffer(unsigned int numberOfBlocks, unsigned int blockSizeInBytes, condition_variable conditionVariables[2]) :
	inCV(conditionVariables), outCV(conditionVariables + 1),
	queue(queueComparator),
	bufferTable(numberOfBlocks)
{
	vertexArrays.insert(vertexArrays.begin(), numberOfBlocks, 0);
	fun()->glGenVertexArrays(numberOfBlocks, vertexArrays.data());

	buffers.insert(buffers.begin(), numberOfBlocks, 0);
	fun()->glGenBuffers(numberOfBlocks, buffers.data());

	for (unsigned int i = 0; i < numberOfBlocks; ++i)
	{
		fun()->glBindVertexArray(vertexArrays[i]);
		fun()->glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

		fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
		//fun()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

		fun()->glEnableVertexAttribArray(0);

		fun()->glBufferData(GL_ARRAY_BUFFER, blockSizeInBytes, nullptr, GL_STATIC_DRAW);
	}

	fun()->glBindVertexArray(0);
}

Buffer::~Buffer()
{
	fun()->glDeleteBuffers(buffers.size(), buffers.data());
	fun()->glDeleteVertexArrays(vertexArrays.size(), vertexArrays.data());
}

#undef fun

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

			queue.pop();

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
		queue.pop();

		return SignalBlock(vertexArrays[lastBuffer], blockIndex);
	}

	return SignalBlock(0, 0);
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
			if (mapP->first == *setP && bufferTable.getNotInUse(mapP->first) == true)
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
			continue; // not needed
		}
		else
		{
			bufferTable.updateLastUsed(bufferIndex);
			bufferTable.setNotInUse(bufferIndex, false);

			return SignalBlock(vertexArrays[bufferIndex], blockFound);
		}
	}

	return SignalBlock(0, 0);
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
