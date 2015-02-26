#include "prioritycachelogic.h"

#include <iostream>

using namespace std;

PriorityCacheLogic::PriorityCacheLogic(unsigned int capacity, int from, int to) :
	capacity(capacity), table(capacity), indexFrom(from), indexTo(to)
{

}

PriorityCacheLogic::~PriorityCacheLogic()
{

}

void PriorityCacheLogic::enqueue(const set<int>& indexSet, int priority)
{
	for (const auto& e : indexSet)
	{
		push(e, priority);
	}
}

bool PriorityCacheLogic::fill(unsigned int* cacheIndex, int* index)
{
	while (queue.empty() == false)
	{
		int topIndex = top().first;
		int topPriority = top().second;

		if (indexMap.count(topIndex) != 0)
		{
			unsigned int ci = indexMap[topIndex];
			table.setPriority(ci, min(table.getPriority(ci), topPriority));
			//table.setPriority(ci, topPriority);
			pop();
		}
		else
		{
			unsigned int lastCacheIndex = table.getLast();
			//table.printTable(cacheIndexMap);

			if (table.getInUse(lastCacheIndex) || table.getPriority(lastCacheIndex) <= topPriority)
			{
				return false;
			}

			pop();

			if (cacheIndexMap.count(lastCacheIndex) != 0)
			{
				int previousIndex = cacheIndexMap[lastCacheIndex];
				indexMap.erase(previousIndex);
			}
			indexMap[topIndex] = lastCacheIndex;
			cacheIndexMap[lastCacheIndex] = topIndex;
			assert(indexMap.size() == cacheIndexMap.size());

			table.setInUse(lastCacheIndex, true);
			table.setPriority(lastCacheIndex, topPriority);
			table.updateLastUsed(lastCacheIndex);

			*cacheIndex = lastCacheIndex;
			*index = topIndex;
			return true;
		}
	}

	return false;
}

bool PriorityCacheLogic::readAny(const set<int>& indexSet, unsigned int* cacheIndex, int* index)
{
	// Try to find if any block is already buffered.
	auto mapP = indexMap.begin();
	auto setP = indexSet.begin();
	int indexFound = -1;

	while (mapP != indexMap.end() && setP != indexSet.end())
	{
		if (mapP->first == *setP && table.getInUse(mapP->second) == false)
		{
			indexFound = mapP->first;
			*cacheIndex = mapP->second;
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

	if (indexFound == -1)
	{
		enqueue(indexSet, -1);
		return false;
	}
	else
	{
		table.updateLastUsed(*cacheIndex);
		table.setInUse(*cacheIndex, true);

		*index = indexFound;
		return true;
	}
}

void PriorityCacheLogicTable::printTable(const map<unsigned int, int>& cacheIndexMap) const
{
#ifndef NDEBUG
	//fprintf(stderr, "Buffer Table:\n");

	fprintf(stderr, "|");
	fprintf(stderr, "   index    inUse priority lastUsed\n"); // 8 chars per collumn

	for (unsigned int i = 0; i < order.size(); ++i)
	{
		fprintf(stderr, "|");
		unsigned int cacheIndex = order[i];
		if (cacheIndexMap.count(cacheIndex) == 0)
		{
			fprintf(stderr, "   empty ");
		}
		else
		{
			fprintf(stderr, "%8d ", cacheIndexMap.at(cacheIndex));
		}
		fprintf(stderr, "%8s ", getInUse(cacheIndex) ? "true" : "false");
		if (getPriority(cacheIndex) == numeric_limits<int>::max())
		{
			fprintf(stderr, "     inf ");
		}
		else
		{
			fprintf(stderr, "%8d ", getPriority(cacheIndex));
		}
		fprintf(stderr, "%8u", lastUsed[cacheIndex]);
		fprintf(stderr, "\n");
	}

	fflush(stderr);
#endif
}

void PriorityCacheLogic::printInfo() const
{
	table.printTable(cacheIndexMap);

	for (const auto& e : queue)
	{
		cerr << "(" << e.first << ", " << e.second << ") ";
	}
	cerr << endl;
}
