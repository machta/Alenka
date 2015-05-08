#ifndef SIGNALBLOCK_H
#define SIGNALBLOCK_H

#include "../openglinterface.h"
#include "CL/cl_gl.h"

#include <cinttypes>

/**
 * @brief A wrapper for the vertex array objects passed from SignalProcessor to Canvas.
 *
 * The order of the vertex array objects is important as each is used in different
 * circumstances:
 *
 * 1. The first array is used for single-channel events when the mode for better quality is active.
 * The number of samples is doubled, every even sample has its duplicate in the following index.
 *
 * 2. The second array skips these duplicate samples.
 *
 * Only one buffer is used to store the actual data.
 */
class SignalBlock
{
public:
	SignalBlock(unsigned int index, int64_t firstSample, int64_t lastSample, GLuint vertexArrays[2]) :
		index(index), firstSample(firstSample), lastSample(lastSample)
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
	int64_t firstSample;
	int64_t lastSample;
	GLuint vertexArrays[2];
};

#endif // SIGNALBLOCK_H
