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
#include <iostream>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, unsigned int memory = PROGRAM_OPTIONS["gpuMemorySize"].as<unsigned int>());
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

		cl_event readyEvent = clCreateUserEvent(clContext->getCLContext(), &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

		std::cerr << "Create dummy event " << readyEvent << std::endl;

		cache->getAny(std::set<int> {index}, nullptr, readyEvent);

		err = clReleaseEvent(readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	unsigned int getCapacity() const
	{
		return cache->getCapacity();
	}

private:
	//DataFile* dataFile;
	GLuint vertexArray;
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
