#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../openglinterface.h"

#include "signalblock.h"
#include "../DataFile/datafile.h"
#include "../options.h"
#include "gpucache.h"
#include "../openclcontext.h"
#include "filterprocessor.h"
#include "montageprocessor.h"

#include <CL/cl_gl.h>

#include <cinttypes>
#include <set>
#include <vector>
#include <sstream>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, unsigned int memory = PROGRAM_OPTIONS["memoryBuffersSize"].as<unsigned int>());
	~SignalProcessor();

	int64_t getBlockSize() const
	{
		return blockSize;
	}

	// ..

	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void prepareBlocks(int index)
	{
		cl_int err;
		cl_mem buffer;
		cl_event readyEvent, doneEvent;

		cache->getAny(std::set<int> {index}, &buffer, &readyEvent, &doneEvent);

		err = clReleaseEvent(readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");

		err = clSetUserEventStatus(doneEvent, CL_COMPLETE);
		checkErrorCode(err, CL_SUCCESS, "clSetUserEventStatus()");
	}
	unsigned int getCapacity() const
	{
		return cache->getCapacity();
	}

private:
	//DataFile* dataFile;
	GLuint vertexArray;
	GLuint buffer;
	OpenCLContext* clContext;
	FilterProcessor* filterProcessor;
	Filter* filter;
	MontageProcessor* montageProcessor;
	Montage* montage;
	GPUCache* cache;

	int M;
	int offset;
	int delay;
	int padding;

	unsigned int blockSize;
	unsigned int cacheBlockSize;
	unsigned int processorTmpBlockSize;
	unsigned int processorOutputBlockSize;

	std::condition_variable processorInCV;
	cl_command_queue processorQueue;
	cl_mem processorTmpBuffer;
	cl_mem processorOutputBuffer;
	GLuint processorVertexArray;

	std::string indexSetString(const std::set<int>& indexSet)
	{
		std::stringstream ss;

		for (const auto& e : indexSet)
		{
			if (e != *indexSet.begin())
			{
				ss << ", ";
			}
			ss << e;
		}

		return ss.str();
	}
};

#endif // SIGNALPROCESSOR_H
