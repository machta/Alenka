#include "kernelcache.h"

#include "../error.h"
#include "../options.h"

#include <pugixml.hpp>

#include <QDir>
#include <QCoreApplication>

using namespace std;
using namespace pugi;

namespace
{

string cacheFilePath()
{
	int platform = PROGRAM_OPTIONS["clPlatform"].as<int>();
	int device = PROGRAM_OPTIONS["clDevice"].as<int>();

	string str = PROGRAM_OPTIONS["kernelCacheDir"].as<string>();
	if (str.empty())
		str = QCoreApplication::applicationDirPath().toStdString();

	return str + QDir::separator().toLatin1() + "kernel-cache-" + to_string(platform) + '-' + to_string(device) + ".xml";
}

} // namespace

KernelCache::KernelCache()
{
	// TODO: Perhaps switch to byte-size based capacity system.
	cache.setMaxCost(PROGRAM_OPTIONS["kernelCacheSize"].as<int>());

	// Load  from a file.
	xml_document xml;
	string filePath = cacheFilePath();
	if (!xml.load_file(filePath.c_str()))
	{
		logToFileAndConsole("Failed to open kernel cache at " << filePath << ".");
	}

	xml_node node = xml.child("cache");
	int size = node.attribute("size").as_int();

	if (size <= 0)
		return;

	xml_node item = node.child("item");
	while (item)
	{
		QString code = QString::fromStdString(item.child("code").text().as_string());
		string binary = item.child("binary").text().as_string();

		if (code.isEmpty() || binary.empty())
			continue;

		auto data = new vector<unsigned char>();
		for (char c : binary)
			data->push_back(c);

		cache.insert(code, data);

		item = item.next_sibling("item");
	}
}

KernelCache::~KernelCache()
{
	// Save to a file.
	int size = cache.size();
	if (size <= 0)
		return;

	xml_document xml;

	xml_node node = xml.append_child("cache");
	node.append_attribute("size").set_value(size);

	const auto& keys = cache.keys();
	for (auto e : keys)
	{
		xml_node item = node.append_child("item");
		item.append_child("code").append_child(node_pcdata).set_value(e.toStdString().c_str());

		std::vector<unsigned char> binary(*find(e));
		binary.push_back(0);

		item.append_child("binary").append_child(node_pcdata).set_value(reinterpret_cast<char*>(binary.data()));
	}

	string filePath = cacheFilePath();
	if (!xml.save_file(filePath.c_str()))
	{
		logToFileAndConsole("Failed to save kernel cache to " << filePath << ".");
	}
}
