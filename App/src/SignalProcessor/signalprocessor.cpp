#include "signalprocessor.h"

using namespace std;

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory, double bufferRatio) : dataFile(file)
{
	unsigned int rawBufferBlockSize = getBlockSize()*file->getChannelCount();

	tmpBuffer = new float[rawBufferBlockSize];

	rawBuffer = new Buffer(memory/rawBufferBlockSize/sizeof(float), cvs.data());

	rawBufferFillerThread = thread(&SignalProcessor::rawBufferFiller, this);

	fun(); // Initialization needs to happen on thread #0.
}

SignalProcessor::~SignalProcessor()
{
	// First finish all secondary threads.
	threadsStop.store(true);

	thread t(&SignalProcessor::joinThreads, this);

	for (auto& e : cvs)
	{
		e.notify_all();
	}

	t.join();

	// Release resources.
	delete rawBuffer;
	delete[] tmpBuffer;
}

SignalBlock SignalProcessor::getAnyBlock(const set<unsigned int>& index)
{
	SignalBlock sb = rawBuffer->readAnyBlock(index, &threadsStop);

	int64_t from = sb.getIndex()*getBlockSize(),
			to = from + getBlockSize() - 1;

	return SignalBlock(sb.geGLBuffer(), sb.getIndex(), dataFile->getChannelCount(), from, to);
}

void SignalProcessor::rawBufferFiller()
{
	while (threadsStop.load() == false)
	{
		SignalBlock sb = rawBuffer->fillBuffer(&threadsStop);

		fun()->glBindBuffer(GL_ARRAY_BUFFER, sb.geGLBuffer());

		int64_t from = sb.getIndex()*getBlockSize(),
				to = from + getBlockSize() - 1;

		dataFile->readData(tmpBuffer, from, to);

		size_t size = getBlockSize()*dataFile->getChannelCount()*sizeof(float);
		fun()->glBufferData(GL_ARRAY_BUFFER, size, tmpBuffer, GL_STATIC_DRAW);

		rawBuffer->release(sb);
	}
}

