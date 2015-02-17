#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../openglinterface.h"

#include "signalblock.h"
#include "../DataFile/datafile.h"
#include "../options.h"
#include "buffer.h"

#include <QOffscreenSurface>

#include <cinttypes>
#include <set>
#include <array>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, unsigned int memory = PROGRAM_OPTIONS->get("memoryBuffersSize").as<unsigned int>(), double bufferRatio = 1);
	~SignalProcessor();

	int64_t getBlockSize()
	{
		return PROGRAM_OPTIONS->get("blockSize").as<unsigned int>();
	}

	// ..

	SignalBlock getAnyBlock(const std::set<unsigned int>& index);
	void release(const SignalBlock& block)
	{
		rawBuffer->release(block);
	}
	void release(const SignalBlock& block, int newPriority)
	{
		rawBuffer->release(block, newPriority);
	}
	void prepareBlocks(const std::set<unsigned int>& index, int priority)
	{
		rawBuffer->enqueue(index, priority);
	}

private:
	std::mutex mtx;

	DataFile* dataFile;
	Buffer* rawBuffer;
	float* rawBufferThreadTmp;
	std::array<std::condition_variable, 2> cvs;
	std::atomic<bool> threadsStop{false};
	std::thread rawBufferFillerThread;
	QOffscreenSurface rawBufferDummySurface;

	void rawBufferFiller(std::atomic<bool>* stop, QOpenGLContext* parentContext);
	void joinThreads()
	{
		rawBufferFillerThread.join();
	}
};

#endif // SIGNALPROCESSOR_H
