#ifndef PRIORITYCACHELOGIC_H
#define PRIORITYCACHELOGIC_H

#include <cstdio>
#include <queue>
#include <map>
#include <set>
#include <limits>
#include <cassert>

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

	void printTable(const std::map<unsigned int, int>& cacheIndexMap);

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

private:
	struct QueueComparator
	{
		bool operator()(std::pair<unsigned int, int> a, std::pair<unsigned int, int> b)
		{
			return a.second > b.second;
		}
	};
	std::priority_queue<std::pair<int, int>, std::vector<std::pair<unsigned int, int>>, QueueComparator> queue;

	std::map<int, unsigned int> indexMap;
	std::map<unsigned int, int> cacheIndexMap;

	PriorityCacheLogicTable table;
	int indexFrom;
	int indexTo;
};

#endif // PRIORITYCACHELOGIC_H
