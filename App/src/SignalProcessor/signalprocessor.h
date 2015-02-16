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
	SignalProcessor(DataFile* file, unsigned int memory = 1*1024*1024, double bufferRatio = 1);
	~SignalProcessor();

	int64_t getBlockSize()
	{
		return PROGRAM_OPTIONS->get("blockSize").as<int>();
	}

	// ..

	SignalBlock getAnyBlock(const std::set<unsigned int>& index);

private:
	std::mutex mtx;

	DataFile* dataFile;
	Buffer* rawBuffer;
	float* tmpBuffer;
	std::array<std::condition_variable, 2> cvs;
	std::atomic<bool> threadsStop = false;
	std::thread rawBufferFillerThread;
	QOffscreenSurface rawBufferDummySurface;

	void rawBufferFiller(std::atomic<bool>* stop, QOpenGLContext* parentContext);
	void joinThreads()
	{
		rawBufferFillerThread.join();
	}
};

#endif // SIGNALPROCESSOR_H
