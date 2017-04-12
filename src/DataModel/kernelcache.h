#ifndef KERNELCACHE_H
#define KERNELCACHE_H

#include <QCache>

#include <vector>

class KernelCache
{
	QCache<QString, std::vector<unsigned char>> cache;

public:
	KernelCache();
	~KernelCache();

	void insert(const QString& code, std::vector<unsigned char>* binary)
	{
		cache.insert(code, binary);
	}
	const std::vector<unsigned char>* find(const QString& code) const
	{
		return cache[code];
	}

	static void deleteCacheFile();
};

#endif // KERNELCACHE_H
