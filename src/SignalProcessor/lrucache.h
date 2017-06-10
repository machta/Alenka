#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <vector>

template <class T> class LRUCacheAllocator {
public:
  virtual ~LRUCacheAllocator() = default;

  virtual bool constructElement(T **ptr) = 0;
  virtual void destroyElement(T *ptr) = 0;
};

template <class K, class T> class LRUCache {
  unsigned int capacity;
  std::unique_ptr<LRUCacheAllocator<T>> allocator;
  std::vector<T *> elements;
  std::vector<unsigned int> lastUsed;
  std::map<K, unsigned int> keyMap;
  std::map<unsigned int, K> reverseKeyMap;

public:
  LRUCache(unsigned int capacity,
           std::unique_ptr<LRUCacheAllocator<T>> allocator)
      : capacity(capacity), allocator(std::move(allocator)) {
    elements.resize(capacity, nullptr);
    lastUsed.resize(capacity);

    for (unsigned int i = 0; i < capacity; ++i)
      lastUsed[i] = 0;
  }
  ~LRUCache() {
    for (auto e : elements)
      allocator->destroyElement(e);
  }

  T *getAny(const std::set<K> &keys, K *key) {
    int keyFound;
    unsigned int cacheIndex;
    T *ret = nullptr;

    if (findFirstKey(keys, &keyFound, &cacheIndex)) {
      ret = elements[keyMap[keyFound]];
      assert(ret);

      updateLastUsed(cacheIndex);

      *key = keyFound;
    }

    return ret;
  }

  T *setOldest(K key) {
    unsigned int maxElement = emptyOrOldest();

    if (!elements[maxElement]) {
      T *ptr;

      if (allocator->constructElement(&ptr))
        elements[maxElement] = ptr;
      else
        maxElement = oldestAlreadyCreated();
    }

    insertKey(key, maxElement);
    updateLastUsed(maxElement);

    assert(elements[maxElement]);
    return elements[maxElement];
  }

  unsigned int getCapacity() const { return capacity; }

  void clear() {
    keyMap.clear();
    reverseKeyMap.clear();
  }

private:
  /**
   * @brief Find a element with key from keys.
   * @param index [out]
   * @param cacheIndex [out]
   * @return True if a common element was found.
   */
  bool findFirstKey(const std::set<K> &keySet, int *key,
                    unsigned int *cacheIndex) {
    auto cacheKeys = keyMap.begin();
    auto keys = keySet.begin();

    while (cacheKeys != keyMap.end() && keys != keySet.end()) {
      if (cacheKeys->first == *keys) {
        *key = cacheKeys->first;
        *cacheIndex = cacheKeys->second;
        return true;
      }

      if (cacheKeys->first < *keys)
        ++cacheKeys;
      else
        ++keys;
    }

    return false;
  }

  unsigned int emptyOrOldest() {
    unsigned int maxElement = 0;
    unsigned int max = lastUsed[maxElement];

    for (unsigned int i = 0; i < capacity; ++i) {
      if (!elements[i])
        return i; // Return early if an empty slot is found.

      if (max < lastUsed[i]) {
        maxElement = i;
        max = lastUsed[i];
      }
    }

    return maxElement;
  }

  unsigned int oldestAlreadyCreated() {
    unsigned int maxElement = 0;

    while (maxElement < capacity - 1 && !elements[maxElement])
      ++maxElement;

    unsigned int max = lastUsed[maxElement];

    for (unsigned int i = maxElement + 1; i < capacity; ++i) {
      if (elements[i] && max < lastUsed[i]) {
        maxElement = i;
        max = lastUsed[i];
      }
    }

    return maxElement;
  }

  void updateLastUsed(unsigned cacheIndex) {
    for (auto &e : lastUsed)
      ++e;

    lastUsed[cacheIndex] = 0;
  }

  void insertKey(unsigned int key, unsigned int cacheIndex) {
    if (reverseKeyMap.count(cacheIndex))
      keyMap.erase(reverseKeyMap[cacheIndex]);

    if (keyMap.count(key))
      reverseKeyMap.erase(keyMap[key]);

    keyMap[key] = cacheIndex;
    reverseKeyMap[cacheIndex] = key;

    assert(keyMap.size() == reverseKeyMap.size());
    assert(keyMap.size() <= capacity);
  }
};

#endif // LRUCACHE_H
