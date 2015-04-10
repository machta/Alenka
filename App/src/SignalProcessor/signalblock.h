#ifndef SIGNALBLOCK_H
#define SIGNALBLOCK_H

#include "../openglinterface.h"
#include "CL/cl_gl.h"

#include <cinttypes>

class SignalBlock
{
public:
	SignalBlock(unsigned int index, int64_t firstSample, int64_t lastSample, GLuint vertexArrays[2]) :
		index(index), channelCount(channelCount), firstSample(firstSample), lastSample(lastSample)
	{
		this->vertexArrays[0] = vertexArrays[0];
		this->vertexArrays[1] = vertexArrays[1];
	}
	~SignalBlock() {}

	GLuint getArray() const
	{
		return vertexArrays[0];
	}
	GLuint getArrayStrideTwo() const
	{
		return vertexArrays[1];
	}
	unsigned int getIndex() const
	{
		return index;
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
	GLuint vertexArrays[2];
};

#endif // SIGNALBLOCK_H
