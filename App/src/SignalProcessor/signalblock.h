#ifndef SIGNALBLOCK_H
#define SIGNALBLOCK_H

#include "../openglinterface.h"
#include "CL/cl_gl.h"

#include <cinttypes>
#include <atomic>

class SignalBlock
{
public:
	SignalBlock(unsigned int index, unsigned int channelCount, int64_t firstSample, int64_t lastSample, GLuint vertexArray) :
		index(index), channelCount(channelCount), firstSample(firstSample), lastSample(lastSample), vertexArray(vertexArray)
	{}
	~SignalBlock() {}

	GLuint getGLVertexArray() const
	{
		return vertexArray;
	}
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
	unsigned int index;
	unsigned int channelCount;
	int64_t firstSample;
	int64_t lastSample;
	GLuint vertexArray;
};

#endif // SIGNALBLOCK_H
