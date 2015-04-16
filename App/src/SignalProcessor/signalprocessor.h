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
#include <cassert>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor();
	~SignalProcessor();

	unsigned int getBlockSize() const
	{
		return blockSize;
	}
	int getTrackCount() const
	{
		return trackCount;
	}
	void updateFilter();
	void updateMontage();
	unsigned int getCapacity() const
	{
		if (ready() == false)
		{
			return -1;
		}

		return cache->getCapacity();
	}
	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void prepareBlock(int index)
	{
		// Preparing of blocks is turned off for the moment.
		assert(0);
		return;

		assert(ready());

		cl_int err;

		cl_event readyEvent = clCreateUserEvent(context->getCLContext(), &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

		cache->getAny(std::set<int> {index}, nullptr, readyEvent);

		err = clReleaseEvent(readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	void changeFile(DataFile* file);
	bool ready() const
	{
		return file != nullptr && trackCount > 0;
	}

private:
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	DataFile* file = nullptr;
	OpenCLContext* context;
	FilterProcessor* filterProcessor;
	MontageProcessor* montageProcessor;
	GPUCache* cache;

	bool onlineFilter;
	unsigned int blockSize;
	int trackCount = 0;

	std::condition_variable processorInCV;
	cl_command_queue commandQueue;
	cl_mem processorTmpBuffer;
	cl_mem processorOutputBuffer = nullptr;
	GLuint vertexArrays[2];
	GLuint glBuffer;

	InfoTable* getInfoTable()
	{
		if (infoTable != nullptr)
		{
			return infoTable;
		}
		else
		{
			return &defaultInfoTable;
		}
	}
	void destroyFileRelated();
	std::string indexSetToString(const std::set<int>& indexSet)
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
	void releaseOutputBuffer()
	{
		if (processorOutputBuffer != nullptr)
		{
			cl_int err = clReleaseMemObject(processorOutputBuffer);
			checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

			processorOutputBuffer = nullptr;
		}
	}
};

#endif // SIGNALPROCESSOR_H
