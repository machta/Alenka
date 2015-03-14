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
	void changeFilter(Filter* filter)
	{
		using namespace std;

		if (PROGRAM_OPTIONS.isSet("printFilter"))
		{
			if (PROGRAM_OPTIONS.isSet("printFilterFile"))
			{
				FILE* file = fopen(PROGRAM_OPTIONS["printFilterFile"].as<string>().c_str(), "w");
				checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["printFilterFile"].as<string>() << "' could not be opened for wtiting.");

				filter->printCoefficients(file);

				fclose(file);
			}
			else
			{
				filter->printCoefficients(stderr);
			}
		}

		filterProcessor->change(filter);
	}
	void changeMontage(Montage* montage)
	{
		montageProcessor->change(montage);
	}
	unsigned int getCapacity() const
	{
		return cache->getCapacity();
	}
	SignalBlock getAnyBlock(const std::set<int>& indexSet);
	void prepareBlock(int index)
	{
		cl_int err;

		cl_event readyEvent = clCreateUserEvent(clContext->getCLContext(), &err);
		checkErrorCode(err, CL_SUCCESS, "clCreateUserEvent()");

		cache->getAny(std::set<int> {index}, nullptr, readyEvent);

		err = clReleaseEvent(readyEvent);
		checkErrorCode(err, CL_SUCCESS, "clReleaseEvent()");
	}

private:
	OpenCLContext* clContext;
	FilterProcessor* filterProcessor;
	bool onlineFilter;
	MontageProcessor* montageProcessor;
	GPUCache* cache;
	Filter* filter;
	Montage* montage;

	int M;
	int offset;
	int delay;
	int padding;

	unsigned int blockSize;
	unsigned int cacheBlockSize;
	unsigned int processorTmpBlockSize;
	unsigned int processorOutputBlockSize;

	std::condition_variable processorInCV;
	cl_command_queue commandQueue;
	cl_mem processorTmpBuffer;
	cl_mem processorOutputBuffer;
	GLuint processorVertexArray;

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
};

#endif // SIGNALPROCESSOR_H
