#include "signalprocessor.h"

using namespace std;

SignalProcessor::SignalProcessor(DataFile* file, uint /*memory*/, double /*bufferRatio*/) : OpenGLInterface(), dataFile(file)
{
	fun()->glGenBuffers(1, &buffer);
	//fun->glBindBuffer(GL_ARRAY_BUFFER, buffer);

	tmpBuffer = new float[getBlockSize()*dataFile->getChannelCount()];
}

SignalProcessor::~SignalProcessor()
{
	fun()->glDeleteBuffers(1, &buffer);
	delete[] tmpBuffer;
}

SignalBlock SignalProcessor::getAnyBlock(const set<unsigned int>& index)
{
	fun()->glBindBuffer(GL_ARRAY_BUFFER, buffer);

	unsigned int block = *index.begin();

	int64_t from = block*getBlockSize(),
			to = (block + 1)*getBlockSize() - 1;

	dataFile->readData(tmpBuffer, from, to);

	size_t size = getBlockSize()*dataFile->getChannelCount()*sizeof(float);
	fun()->glBufferData(GL_ARRAY_BUFFER, size, tmpBuffer, GL_STATIC_DRAW);

	return SignalBlock(buffer, block, dataFile->getChannelCount(), from, to);
}

