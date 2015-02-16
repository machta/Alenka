#include "signalprocessor.h"

using namespace std;

SignalProcessor::SignalProcessor(DataFile* file, unsigned int memory, double bufferRatio) : dataFile(file)
{
	unsigned int rawBufferBlockSize = getBlockSize()*file->getChannelCount();

	tmpBuffer = new float[rawBufferBlockSize];

	rawBufferDummySurface.create();
	rawBuffer = new Buffer(memory/rawBufferBlockSize/sizeof(float), cvs.data());
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
	delete[] tmpBuffer;
}

SignalBlock SignalProcessor::getAnyBlock(const set<unsigned int>& index)
{
	SignalBlock sb = rawBuffer->readAnyBlock(index, &threadsStop);

	int64_t from = sb.getIndex()*getBlockSize(),
			to = from + getBlockSize() - 1;

	return SignalBlock(sb.geGLBuffer(), sb.getIndex(), dataFile->getChannelCount(), from, to);
}

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

		local.fun()->glBindBuffer(GL_ARRAY_BUFFER, sb.geGLBuffer());

		int64_t from = sb.getIndex()*getBlockSize(),
				to = from + getBlockSize() - 1;

		dataFile->readData(tmpBuffer, from, to);

		size_t size = getBlockSize()*dataFile->getChannelCount()*sizeof(float);
		local.fun()->glBufferData(GL_ARRAY_BUFFER, size, tmpBuffer, GL_STATIC_DRAW);

		rawBuffer->release(sb);
	}
}

