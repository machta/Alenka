#include <gtest/gtest.h>

#include "../../src/SignalProcessor/lrucache.h"

#include <memory>

using namespace std;

namespace {

class FloatAllocator : public LRUCacheAllocator<float> {
  const int size = 1024;
  int *destroyCounter;

public:
  FloatAllocator(int *destroyedCount = nullptr)
      : destroyCounter(destroyedCount) {}

  bool constructElement(float **ptr) override {
    *ptr = new float[1024];
    return true;
  }
  void destroyElement(float *ptr) override {
    if (destroyCounter && ptr)
      ++(*destroyCounter);
    delete[] ptr;
  }
};

void testKey(int key, float *ptr0, float *ptr1) {
  if (key == 0) {
    EXPECT_EQ(ptr0[0], 0);
    EXPECT_EQ(ptr0[1], 5);
  } else if (key == 1) {
    EXPECT_EQ(ptr1[0], 55);
    EXPECT_EQ(ptr1[1], 100);
  } else {
    FAIL();
  }
}

void randomTest(int cap, int iters, int range, int clearInterval) {
  LRUCache<int, float> cache(cap, make_unique<FloatAllocator>(nullptr));

  for (int i = 0; i < iters; ++i) {
    int r = rand() % range;

    int key = -1;
    float *ptr = cache.getAny(set<int>{r}, &key);

    if (ptr) {
      EXPECT_EQ(key, r);
      EXPECT_EQ(static_cast<int>(*ptr), r);
    } else {
      ptr = cache.setOldest(r);
      *ptr = static_cast<float>(r);
    }

    if (clearInterval && i % clearInterval == 0)
      cache.clear();
  }
}

} // namespace

TEST(lrucache_test, adhoc) {
  int destroyCounter = 0;
  {
    LRUCache<int, float> cache(10,
                               make_unique<FloatAllocator>(&destroyCounter));
    EXPECT_EQ(cache.getCapacity(), static_cast<unsigned int>(10));

    int key0 = -1, key1 = -1;
    EXPECT_FALSE(cache.getAny(set<int>{1}, &key0));

    float *ptr0 = cache.setOldest(0);
    ptr0[0] = 0;
    ptr0[1] = 5;

    float *ptr1 = cache.setOldest(1);
    ptr1[0] = 55;
    ptr1[1] = 100;

    EXPECT_NE(ptr0, ptr1);

    float *secondPointer0;
    float *secondPointer1;

    for (int i = 0; i < 1000; ++i) {
      set<int> keys{0, 1};
      secondPointer0 = cache.getAny(keys, &key0);
      keys.erase(key0);
      secondPointer1 = cache.getAny(keys, &key1);

      EXPECT_NE(secondPointer0, secondPointer1);
      EXPECT_NE(key0, key1);

      testKey(key0, secondPointer0, secondPointer1);
      testKey(key1, secondPointer0, secondPointer1);

      for (int j = 0; j < 7; ++j)
        cache.setOldest(2 + j + i * 20)[0] = 2.f + j;
    }

    EXPECT_FALSE(cache.getAny(set<int>{-1}, &key0));

    EXPECT_TRUE(cache.getAny(set<int>{0}, &key0));
    cache.clear();
    EXPECT_FALSE(cache.getAny(set<int>{0}, &key0));
  }
  EXPECT_EQ(destroyCounter, 10);
}

TEST(lrucache_test, random) {
  srand(5);

  for (int i = 1; i < 4; ++i)
    randomTest(11 * i * i, 1000 * i * i * i, 99 * i * i, 0);
}

TEST(lrucache_test, random_clear) {
  srand(6);
  for (int i = 1; i < 5; ++i)
    randomTest(13 * i * i, 1000 * i * i * i, 111 * i * i, 1000 * i * i);
}
