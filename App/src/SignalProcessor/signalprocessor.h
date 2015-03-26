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

	int64_t getBlockSize() const
	{
		return blockSize;
	}
	void changeFilter(Filter* filter);
	void changeMontage(Montage* montage);
	unsigned int getCapacity() const
	{
		if (noFile)
		{
			return -1;
		}

		return cache->getCapacity();
	}
	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void prepareBlock(int index)
	{
		assert(noFile == false);

		cl_int err;

		cl_event readyEvent = clCreateUserEvent(context->getCLContext(), &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

		cache->getAny(std::set<int> {index}, nullptr, readyEvent);

		err = clReleaseEvent(readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}
	void changeFile(DataFile* file);
	bool ready()
	{
		return !noFile;
	}

private:
	bool noFile = true;
	OpenCLContext* context;
	FilterProcessor* filterProcessor;
	MontageProcessor* montageProcessor;
	GPUCache* cache;

	bool onlineFilter;
	unsigned int blockSize;

	std::condition_variable processorInCV;
	cl_command_queue commandQueue;
	cl_mem processorTmpBuffer;
	cl_mem processorOutputBuffer = nullptr;
	GLuint processorVertexArray;

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
	void deleteOutputBuffer()
	{
		if (processorOutputBuffer != nullptr)
		{
			gl()->glDeleteVertexArrays(1, &processorVertexArray);

			cl_int err = clReleaseMemObject(processorOutputBuffer);
			checkErrorCode(err, CL_SUCCESS, "clReleaseMemObject()");

			processorOutputBuffer = nullptr;
		}
	}
};

#endif // SIGNALPROCESSOR_H
