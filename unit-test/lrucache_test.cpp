#ifdef TESTS

#include <gtest/gtest.h>

#include "../src/SignalProcessor/lrucache.h"

using namespace std;

namespace
{

class FloatAllocator : public LRUCacheAllocator<float>
{
	const int size = 1024;
	int* destroyCounter;

public:
	FloatAllocator(int* destroyedCount = nullptr) : destroyCounter(destroyedCount) {}

	virtual bool constructElement(float** ptr) override
	{
		*ptr = new float[1024];
		return true;
	}
	virtual void destroyElement(float* ptr) override
	{
		if (destroyCounter && ptr)
			++(*destroyCounter);
		delete[] ptr;
	}
};

void testKey(int key, float* ptr0, float* ptr1)
{
	if (key == 0)
	{
		EXPECT_EQ(ptr0[0], 0);
		EXPECT_EQ(ptr0[1], 5);
	}
	else if (key == 1)
	{
		EXPECT_EQ(ptr1[0], 55);
		EXPECT_EQ(ptr1[1], 100);
	}
	else
	{
		FAIL();
	}
}

} // namespace

TEST(lrucache_test, float_cache)
{
	int destroyCounter = 0;
	{
		LRUCache<int, float> cache(10, new FloatAllocator(&destroyCounter));
		EXPECT_EQ(cache.getCapacity(), 10);

		int key0, key1;
		EXPECT_FALSE(cache.getAny(set<int>{1}, &key0));

		float* ptr0 = cache.setOldest(0);
		ptr0[0] = 0;
		ptr0[1] = 5;

		float* ptr1 = cache.setOldest(1);
		ptr1[0] = 55;
		ptr1[1] = 100;

		EXPECT_NE(ptr0, ptr1);

		float* secondPointer0;
		float* secondPointer1;

		for (int i = 0; i < 1000; ++i)
		{
			set<int> keys{0, 1};
			secondPointer0 = cache.getAny(keys, &key0);
			keys.erase(key0);
			secondPointer1 = cache.getAny(keys, &key1);

			EXPECT_NE(secondPointer0, secondPointer1);
			EXPECT_NE(key0, key1);

			testKey(key0, secondPointer0, secondPointer1);
			testKey(key1, secondPointer0, secondPointer1);

			for (int j = 0; j < 7; ++j)
				cache.setOldest(2 + j + i*20)[0] = 2 + j;
		}

		EXPECT_FALSE(cache.getAny(set<int>{-1}, &key0));

		EXPECT_TRUE(cache.getAny(set<int>{0}, &key0));
		cache.clear();
		EXPECT_FALSE(cache.getAny(set<int>{0}, &key0));
	}
	EXPECT_EQ(destroyCounter, 10);
}

#endif // TESTS
