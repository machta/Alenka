#include "signalprocessor.h"

#include <algorithm>
#include <stdexcept>

using namespace std;

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory, double /*bufferRatio*/) : dataFile(file)
{
	unsigned int rawBufferBlockSize = getBlockSize()*file->getChannelCount();
	unsigned int rawBufferBytes = rawBufferBlockSize*sizeof(float);

	//unsigned int rawBufferBlockCounts = max<unsigned int>(1, memory/rawBufferBytes);
	unsigned int rawBufferBlockCount = memory/rawBufferBytes;
	if (rawBufferBlockCount <= 0)
	{
		throw runtime_error("Not enough availible memory for the rawBuffer");
	}

	rawBufferDummySurface.create();
	rawBuffer = new Buffer(rawBufferBlockCount, rawBufferBytes, cvs.data());
	rawBufferThreadTmp = new float[rawBufferBlockSize];
	rawBufferFillerThread = thread(&SignalProcessor::rawBufferFiller, this, &threadsStop, QOpenGLContext::currentContext());
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
	delete[] rawBufferThreadTmp;
}

SignalBlock SignalProcessor::getAnyBlock(const set<unsigned int>& index)
{
	SignalBlock sb = rawBuffer->readAnyBlock(index, &threadsStop);

	int64_t from = sb.getIndex()*getBlockSize(),
			to = from + getBlockSize() - 1;

	return SignalBlock(sb.geVertexArray(), sb.getIndex(), dataFile->getChannelCount(), from, to);
}

#define fun() fun_shortcut()

void SignalProcessor::rawBufferFiller(atomic<bool>* stop, QOpenGLContext* parentContext)
{
	QOpenGLContext context;
	context.setShareContext(parentContext);
	checkErrorCode(context.create(), true, "creating a new OpenGL context");
	checkErrorCode(context.makeCurrent(&rawBufferDummySurface), true, "make this context current");

	OpenGLInterface local;

	while (stop->load() == false)
	{
		SignalBlock sb = rawBuffer->fillBuffer(&threadsStop);

		//local.fun()->glBindBuffer(GL_ARRAY_BUFFER, sb);
		local.fun()->glBindVertexArray(sb.geVertexArray());
		local.fun();

		int64_t from = sb.getIndex()*getBlockSize(),
				to = from + getBlockSize() - 1;

		dataFile->readData(rawBufferThreadTmp, from, to);

		size_t size = getBlockSize()*dataFile->getChannelCount()*sizeof(float);
		local.fun()->glBufferSubData(GL_ARRAY_BUFFER, 0, size, rawBufferThreadTmp);
		//local.fun()->glBufferData(GL_ARRAY_BUFFER, size, tmpBuffer, GL_STATIC_DRAW);

		rawBuffer->release(sb);

		fun()->glBindVertexArray(0);
	}
}

#undef fun
