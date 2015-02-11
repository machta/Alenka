#ifndef SIGNALBLOCK_H
#define SIGNALBLOCK_H

#include "../openglinterface.h"

#include <cinttypes>

class SignalBlock
{
public:
	SignalBlock(GLuint buffer, unsigned int index, unsigned int channelCount, int64_t firstSample, int64_t lastSample) :
		buffer(buffer), index(index), channelCount(channelCount), firstSample(firstSample), lastSample(lastSample) {}
	~SignalBlock() {}

	GLuint geGLBuffer() const
	{
		return buffer;
	}
	//... geCLBuffer(){}

	unsigned int getIndex() const
	{
		return index;
	}
	unsigned int getchannelCount() const
	{
		return channelCount;
	}
	int64_t getFirstSample() const
	{
		return firstSample;
	}
	int64_t getLastSample() const
	{
		return lastSample;
	}

private:
	GLuint buffer;
	unsigned int index;
	unsigned int channelCount;
	int64_t firstSample;
	int64_t lastSample;
};

#endif // SIGNALBLOCK_H
