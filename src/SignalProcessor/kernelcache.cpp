#include "kernelcache.h"

#include <AlenkaSignal/montage.h>

void KernelCache::shrink(int capacity)
{
	for (int i = map.size() - capacity; i > 0; i--)
	{
		auto montage = map.begin()->second;
		delete montage;
		map.erase(map.begin());
	}
}
