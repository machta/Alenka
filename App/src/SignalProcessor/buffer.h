#ifndef BUFFER_H
#define BUFFER_H

#include "../openglinterface.h"

#include "signalblock.h"

#include <algorithm>
#include <set>
#include <array>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <utility>

class Buffer : public OpenGLInterface
{
public:
	Buffer(unsigned int numberOfBlocks, unsigned int blockSizeInBytes, std::condition_variable conditionVariables[2]);
	~Buffer();

	void clearBuffer();
	void enqueue(const std::set<unsigned int>& index, int priority);
	SignalBlock fillBuffer(std::atomic<bool>* stop);
	SignalBlock readAnyBlock(const std::set<unsigned int>& index, std::atomic<bool>* stop);
	void release(const SignalBlock& block);
	void release(const SignalBlock& block, int newPriority);

private:
	std::mutex mtx;
	std::condition_variable* inCV;
	std::condition_variable* outCV;

	std::vector<GLuint> vertexArrays;
	std::vector<GLuint> buffers;

	struct QueueComparator
	{
		bool operator()(std::pair<unsigned int, int> a, std::pair<unsigned int, int> b)
		{
			return a.second > b.second;
		}
	} queueComparator;
	std::priority_queue<std::pair<unsigned int, int>, std::vector<std::pair<unsigned int, int>>, QueueComparator> queue;

	std::map<unsigned int, unsigned int> blockBufferMap;

	struct Table
	{
	private:
		bool dirty = true;
		std::vector<unsigned int> order;
		std::vector<bool> notInUse;
		std::vector<int> priority;
		std::vector<unsigned int> lastUsed;

	public:
		Table(unsigned int blocks)
		{
			order.reserve(blocks);
			for (unsigned int i = 0; i < blocks; ++i)
			{
				order.push_back(i);
			}

			notInUse.insert(notInUse.begin(), blocks, true);
			priority.insert(priority.begin(), blocks, 0);
			lastUsed.insert(lastUsed.begin(), blocks, 0);
		}

		unsigned int getOrder(int i)
		{
			if (dirty)
			{
				dirty = false;

				auto predicate = [this] (unsigned int a, unsigned int b) -> bool
				{
					if (this->getNotInUse(a) < getNotInUse(b))
					{
						return true;
					}
					else if (this->getNotInUse(a) > this->getNotInUse(b))
					{
						return false;
					}

					if (this->getPriority(a) < this->getPriority(b))
					{
						return true;
					}
					else if (this->getPriority(a) > this->getPriority(b))
					{
						return false;
					}

					if (this->getLastUsed(a) < this->getLastUsed(b))
					{
						return true;
					}
					return false;
				};

				std::sort(order.begin(), order.end(), predicate);
			}
			return order[i];
		}
		unsigned int getLastOrder()
		{
			return getOrder(order.size() - 1);
		}
		//void setOrder(int i, unsigned int val) { order[i] = val; }

		bool getNotInUse(int i) const
		{
			return notInUse[i];
		}
		void setNotInUse(int i, bool val)
		{
			dirty = true;
			notInUse[i] = val;
		}

		int getPriority(int i) const
		{
			return priority[i];
		}
		void setPriority(int i, int val)
		{
			dirty = true;
			priority[i] = val;
		}

		unsigned int getLastUsed(int i) const
		{
			return lastUsed[i];
		}
		//		void setLastUsed(int i, unsigned int val)
		//		{
		//			dirty = true;
		//			lastUsed[i] = val;
		//		}
		void updateLastUsed(int i)
		{
			dirty = true;

			for (auto& e : lastUsed)
			{
				e++;
			}
			lastUsed[i] = 0;
		}
	} bufferTable;

	unsigned int releaseCommonPart(const SignalBlock& block)
	{
		inCV->notify_one();
		outCV->notify_one();

		unsigned int bufferIndex = blockBufferMap.at(block.getIndex());
		bufferTable.setNotInUse(bufferIndex, true);

		return bufferIndex;
	}

};

#endif // BUFFER_H
