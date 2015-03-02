#ifndef PRIORITYCACHELOGIC_H
#define PRIORITYCACHELOGIC_H

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
#include <map>
#include <set>

class PriorityCacheLogicTable
{
public:
	PriorityCacheLogicTable(unsigned int size)
	{
		order.reserve(size);
		for (unsigned int i = 0; i < size; ++i)
		{
			order.push_back(i);
		}

		notInUse.insert(notInUse.begin(), size, true);
		priority.insert(priority.begin(), size, std::numeric_limits<int>::max());
		lastUsed.insert(lastUsed.begin(), size, 0);
	}

	unsigned int getLast()
	{
		if (dirty)
		{
			dirty = false;

			auto predicate = [this] (unsigned int a, unsigned int b) /*-> bool*/
			{
				if (notInUse[a] < notInUse[b])
				{
					return true;
				}
				else if (notInUse[a] > notInUse[b])
				{
					return false;
				}

				if (priority[a] < priority[b])
				{
					return true;
				}
				else if (priority[a] > priority[b])
				{
					return false;
				}

				if (lastUsed[a] < lastUsed[b])
				{
					return true;
				}
				return false;
			};

			std::sort(order.begin(), order.end(), predicate);
		}

		return order[order.size() - 1];
	}
	bool getInUse(unsigned int i) const
	{
		return !notInUse[i];
	}
	void setInUse(unsigned int i, bool val)
	{
		dirty = true;
		notInUse[i] = !val;
	}
	int getPriority(unsigned int i) const
	{
		return priority[i];
	}
	void setPriority(unsigned int i, int val)
	{
		dirty = true;
		priority[i] = val;
	}
	void updateLastUsed(unsigned int i)
	{
		dirty = true;

		for (auto& e : lastUsed)
		{
			e++;
		}
		lastUsed[i] = 0;
	}

	void printTable(const std::map<unsigned int, int>& cacheIndexMap) const;

private:
	bool dirty = true;
	std::vector<unsigned int> order;
	std::vector<bool> notInUse;
	std::vector<int> priority;
	std::vector<unsigned int> lastUsed;
};

class PriorityCacheLogic
{
public:
	PriorityCacheLogic(unsigned int capacity, int from = 0, int to = 0);
	~PriorityCacheLogic();

	void enqueue(const std::set<int>& indexSet, int priority);
	bool fill(unsigned int* cacheIndex, int* index);
	bool read(int index, unsigned int* cacheIndex)
	{
		int oldIndex = index;

		bool ret = readAny(std::set<int> {index}, cacheIndex, &index);

		assert(index == oldIndex);

		return ret;
	}
	bool readAny(const std::set<int>& indexSet, unsigned int* cacheIndex, int* index);
	unsigned int release(int index)
	{
		unsigned int cacheIndex = indexMap.at(index);

		assert(table.getInUse(cacheIndex) == true);
		table.setInUse(cacheIndex, false);

		return cacheIndex;
	}
	unsigned int release(int index, int newPriority)
	{
		unsigned int cacheIndex = release(index);
		table.setPriority(cacheIndex, newPriority);
		return cacheIndex;
	}
	void clear() {}
	void printInfo() const;

private:
	struct QueueComparator
	{
		bool operator()(std::pair<int, int> a, std::pair<int, int> b)
		{
			return a.second > b.second;
		}
	} queueComparator;
	std::vector<std::pair<int, int>> queue;

	std::map<int, unsigned int> indexMap;
	std::map<unsigned int, int> cacheIndexMap;

	unsigned int capacity;
	PriorityCacheLogicTable table;
	int indexFrom;
	int indexTo;

	void push(int val, int priority)
	{
#ifdef NDEBUG
		if (queue.size() >= 100*capacity)
		{
			queue.back() = std::make_pair(val, priority);
		}
		else
#endif
		{
			queue.push_back(std::make_pair(val, priority));
		}

#ifdef NDEBUG
		std::push_heap(queue.begin(), queue.end(), queueComparator);
#else
		std::sort(queue.begin(), queue.end(), queueComparator);
		std::reverse(queue.begin(), queue.end());
#endif

		//if (is_heap(queue.begin(), queue.end(), queueComparator) == false) printInfo();
		assert(is_heap(queue.begin(), queue.end(), queueComparator));
	}
	void pop()
	{
		assert(queue.empty() == false);

#ifdef NDEBUG
		std::pop_heap(queue.begin(), queue.end(), queueComparator);
		queue.pop_back();
#else
		std::swap(queue.front(), queue.back());
		queue.pop_back();
		std::sort(queue.begin(), queue.end(), queueComparator);
		std::reverse(queue.begin(), queue.end());
#endif

		//if (is_heap(queue.begin(), queue.end(), queueComparator) == false) printInfo();
		assert(is_heap(queue.begin(), queue.end(), queueComparator));
	}
	const std::pair<int, int>& top() const
	{
		return queue[0];
	}
};

#endif // PRIORITYCACHELOGIC_H
