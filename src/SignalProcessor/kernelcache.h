#ifndef KERNELCACHE_H
#define KERNELCACHE_H

#include <string>
#include <unordered_map>
#include <utility>

namespace AlenkaSignal
{
template<class T>
class Montage;
}

class KernelCache
{
public:
	KernelCache() {}

	void add(const std::string& code, AlenkaSignal::Montage<float>* montage)
	{
		auto pair = std::make_pair(code, montage);
		map.insert(pair);
	}
	AlenkaSignal::Montage<float>* find(const std::string& code) const
	{
		auto it = map.find(code);
		return it == map.end() ? nullptr : it->second;
	}
	int size() const
	{
		return map.size();
	}
	void shrink(int capacity);

private:
	std::unordered_map<std::string, AlenkaSignal::Montage<float>*> map;
};

#endif // KERNELCACHE_H
