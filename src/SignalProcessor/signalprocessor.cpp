#include "signalprocessor.h"

#include "../signalfilebrowserwindow.h"
#include <AlenkaFile/datafile.h>
#include "../myapplication.h"
#include "kernelcache.h"
#include "signalblock.h"

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filter.h>
#include <AlenkaSignal/filterprocessor.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaSignal/montageprocessor.h>

#include <algorithm>
#include <stdexcept>
#include <chrono>

#include <QFile>

using namespace std;
using namespace AlenkaFile;

namespace
{

AlenkaSignal::WindowFunction resolveWindow()
{
	string window = PROGRAM_OPTIONS["window"].as<string>();

	if (window == "hamming")
		return AlenkaSignal::WindowFunction::Hamming;
	else if (window == "blackman")
		return AlenkaSignal::WindowFunction::Blackman;

	return AlenkaSignal::WindowFunction::None;
}

AbstractTrackTable* getTrackTable(DataFile* file)
{
	return file->getDataModel().montageTable->trackTable(SignalFileBrowserWindow::infoTable.getSelectedMontage());
}

} // namespace

SignalProcessor::SignalProcessor()
{
	onlineFilter = PROGRAM_OPTIONS["onlineFilter"].as<bool>();
	glSharing = PROGRAM_OPTIONS["glSharing"].as<bool>();

	cl_int err;

	initializeOpenGLInterface();

	if (glSharing)
	{
		context = new AlenkaSignal::OpenCLContext(PROGRAM_OPTIONS["clPlatform"].as<int>(), PROGRAM_OPTIONS["clDevice"].as<int>(), true);
	}
	else
	{
		context = globalContext.get();
	}

	commandQueue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
	checkClErrorCode(err, "clCreateCommandQueue()");

	gl()->glGenBuffers(1, &glBuffer);
	glGenVertexArrays(2, vertexArrays);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);

	glBindVertexArray(vertexArrays[0]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	glBindVertexArray(vertexArrays[1]);
	gl()->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2*sizeof(float), reinterpret_cast<void*>(0));
	gl()->glEnableVertexAttribArray(0);

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	gl();

	kernelCache = new KernelCache();
	QFile headerFile(":/montageHeader.cl");
	headerFile.open(QIODevice::ReadOnly);
	header = headerFile.readAll().toStdString();
}

SignalProcessor::~SignalProcessor()
{
	cl_int err;

	deleteMontage();
	destroyFileRelated();

	if (glSharing)
		delete context;

	err = clReleaseCommandQueue(commandQueue);
	checkClErrorCode(err, "clReleaseCommandQueue()");

	gl()->glDeleteBuffers(1, &glBuffer);
	glDeleteVertexArrays(2, vertexArrays);

	gl();

	kernelCache->shrink(0);
	assert(kernelCache->size() == 0 && "Make sure the kernel cache is empty.");
	delete kernelCache;
}

void SignalProcessor::updateFilter()
{
	using namespace std;

	if (!file)
		return;

	int M = file->getSamplingFrequency()/* + 1*/;
	AlenkaSignal::Filter<float> filter(M, file->getSamplingFrequency()); // Possibly could save this object so that it won't be created from scratch everytime.

	filter.lowpass(true);
	filter.setLowpass(SignalFileBrowserWindow::infoTable.getLowpassFrequency());

	filter.highpass(true);
	filter.setHighpass(SignalFileBrowserWindow::infoTable.getHighpassFrequency());

	filter.notch(SignalFileBrowserWindow::infoTable.getNotch());
	filter.setNotch(50);

	filterProcessor->changeSampleFilter(M, filter.computeSamples());

	if (PROGRAM_OPTIONS.isSet("printFilter"))
	{
		if (PROGRAM_OPTIONS.isSet("printFilterFile"))
		{
			FILE* file = fopen(PROGRAM_OPTIONS["printFilterFile"].as<string>().c_str(), "w");
			checkNotErrorCode(file, nullptr, "File '" << PROGRAM_OPTIONS["printFilterFile"].as<string>() << "' could not be opened for writing.");

			filter.printCoefficients(file, filterProcessor->getCoefficients());

			int err = fclose(file);
			checkErrorCode(err, 0, "fclose()");
		}
		else
		{
			filter.printCoefficients(stdout, filterProcessor->getCoefficients());
		}
	}

	if (onlineFilter == false)
	{
		cache->clear();
	}
}

void SignalProcessor::setUpdateMontageFlag()
{
	if (file)
	{
		trackCount = 0;
		for (int i = 0; i < getTrackTable(file)->rowCount(); ++i)
		{
			if (getTrackTable(file)->row(i).hidden == false)
				++trackCount;
		}

		if (trackCount > 0)
			updateMontageFlag = true;
	}
}

SignalBlock SignalProcessor::getAnyBlock(const std::set<int>& indexSet)
{
	assert(ready());
	assert(indexSet.empty() == false);

	if (updateMontageFlag)
	{
		updateMontageFlag = false;
		updateMontage();
	}

	cl_int err;

	cl_event readyEvent = clCreateUserEvent(context->getCLContext(), &err);
	checkClErrorCode(err, "clCreateUserEvent()");

	AlenkaSignal::OpenCLContext::enqueueBarrier(commandQueue, readyEvent);

	int index = cache->getAny(indexSet, processorTmpBuffer, readyEvent);

	printBuffer("after_getAny.txt", processorTmpBuffer, commandQueue);

	if (onlineFilter)
	{
		assert(false && "Fix this later!");
		//filterProcessor->process(processorTmpBuffer, commandQueue);

		printBuffer("after_filter.txt", processorTmpBuffer, commandQueue);

		err = clFlush(commandQueue);
		checkClErrorCode(err, "clFlush()");
	}

	if (glSharing)
	{
		gl()->glFinish(); // Could be replaced by a fence.

		err = clEnqueueAcquireGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueAcquireGLObjects()");
	}

	montageProcessor->process(montage, processorTmpBuffer, processorOutputBuffer, commandQueue);

	printBuffer("after_montage.txt", processorOutputBuffer, commandQueue);

	if (glSharing)
	{
		err = clFinish(commandQueue);
		checkClErrorCode(err, "clFinish()");

		err = clEnqueueReleaseGLObjects(commandQueue, 1, &processorOutputBuffer, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReleaseGLObjects()");

		err = clFinish(commandQueue);
		checkClErrorCode(err, "clFinish()");
	}
	else
	{
		// Pull the data from CL buffer and copy it to the GL buffer.

		unsigned int outputBlockSize = blockSize*trackCount; // Code-duplicity.
		//outputBlockSize *= PROGRAM_OPTIONS["eventRenderMode"].as<int>();

		err = clEnqueueReadBuffer(commandQueue, processorOutputBuffer, CL_TRUE, 0, outputBlockSize*sizeof(float), processorOutputBufferTmp, 0, nullptr, nullptr);
		checkClErrorCode(err, "clGetMemObjectInfo");

		gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		gl()->glBufferData(GL_ARRAY_BUFFER, outputBlockSize*sizeof(float), processorOutputBufferTmp, GL_STATIC_DRAW);
	}

	auto fromTo = GPUCache::blockIndexToSampleRange(index, getBlockSize());
	return SignalBlock(index, fromTo.first, fromTo.second, vertexArrays);
}

void SignalProcessor::changeFile(DataFile* file)
{
	destroyFileRelated();

	this->file = file;

	if (file)
	{
		int M = file->getSamplingFrequency();
		int offset = M;
		int delay = M/2 - 1;
		blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
		unsigned int tmpBlockSize = (blockSize + offset)*file->getChannelCount();

		// Construct the filter and montage processors.
		filterProcessor = new AlenkaSignal::FilterProcessor<float>(blockSize + offset, file->getChannelCount(), context, resolveWindow());

		montageProcessor = new AlenkaSignal::MontageProcessor<float>(offset, blockSize, file->getChannelCount());

		// Construct the cache.
		int64_t memoryToUse = PROGRAM_OPTIONS["gpuMemorySize"].as<int64_t>();
		cl_int err;

		cl_ulong maxMemorySize;
		err = clGetDeviceInfo(context->getCLDevice(), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &maxMemorySize, nullptr);
		checkClErrorCode(err, "clGetDeviceInfo()");

		if (memoryToUse <= 0)
			memoryToUse += maxMemorySize;

		// Ensure the limit is within a reasonable interval.
		memoryToUse = max<int64_t>(memoryToUse, maxMemorySize*0.1);
		memoryToUse = min<int64_t>(memoryToUse, maxMemorySize*0.75);

		memoryToUse -= tmpBlockSize + blockSize*sizeof(float)*2048; // substract the sizes of the tmp buffer and the output buffer (for a realistically big montage)
		assert(memoryToUse > 0 && "There is no memory left for the GPU buffer.");

		cache = new GPUCache(blockSize, offset, delay, memoryToUse, file, context, onlineFilter ? nullptr : filterProcessor);

		// Construct tmp buffer.
		cl_mem_flags flags = CL_MEM_READ_WRITE;
#ifdef NDEBUG
#if CL_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif

		processorTmpBuffer = clCreateBuffer(context->getCLContext(), flags, tmpBlockSize*sizeof(float), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer()");

		// Default filter and montage.
		updateFilter();

		setUpdateMontageFlag();
	}
}

string SignalProcessor::simplifyMontage(const string& str)
{
	QString qstr = AlenkaSignal::Montage<float>::stripComments(str).c_str();
	return qstr.simplified().toStdString();
}

void SignalProcessor::destroyFileRelated()
{
	if (file)
	{
		file = nullptr;

		delete cache;
		delete filterProcessor;
		delete montageProcessor;

		cl_int err = clReleaseMemObject(processorTmpBuffer);
		checkClErrorCode(err, "clReleaseMemObject()");

		releaseOutputBuffer();
	}
}

void SignalProcessor::releaseOutputBuffer()
{
	if (processorOutputBuffer)
	{
		cl_int err = clReleaseMemObject(processorOutputBuffer);
		checkClErrorCode(err, "clReleaseMemObject()");

		processorOutputBuffer = nullptr;
	}

	delete[] processorOutputBufferTmp;
	processorOutputBufferTmp = nullptr;
}

void SignalProcessor::updateMontage()
{
	using namespace std::chrono;
	assert(ready());

#ifndef NDEBUG
	auto start = high_resolution_clock::now(); // TODO: Remove this after the compilation time issue is solved.
#endif

	deleteMontage();

	int capacity = max<int>(PROGRAM_OPTIONS["kernelCacheSize"].as<int>(), getTrackTable(file)->rowCount());
	if (kernelCache->size() > capacity)
	{
		kernelCache->shrink(capacity);
		logToFileAndConsole("Shrinking kernel cache.");
	}
	assert(kernelCache->size() <= capacity);

	vector<int> newMontageIndex;
	vector<string> newMontageCode;

	for (int i = 0; i < getTrackTable(file)->rowCount(); i++)
	{
		Track t = getTrackTable(file)->row(i);

		if (t.hidden)
			continue;

		string str = simplifyMontage(t.code);

		AlenkaSignal::Montage<float>* ptr = kernelCache->find(str);
		if (!ptr)
		{
			newMontageIndex.push_back(i);
			newMontageCode.push_back(str);
		}
		montage.push_back(ptr);
	}

	assert(newMontageIndex.size() == newMontageCode.size());
	mutex mtx;

	// For some reason the compilation gets serialized for an Nvidia card.
	// TODO: Figure out whether this is an OpenCL issue, and if yes, remove the parallel block and simplify this code.
	int iters = static_cast<int>(newMontageIndex.size());
	#pragma omp parallel for
	for (int i = 0; i < iters; i++)
	{
		AlenkaSignal::Montage<float>* m = new AlenkaSignal::Montage<float>(newMontageCode[i], context, header);

		{
			lock_guard<mutex> lock(mtx);
			kernelCache->add(newMontageCode[i], m);
			montage[newMontageIndex[i]] = m;
		}
	}

	assert(static_cast<int>(montage.size()) == trackCount);

#ifndef NDEBUG
	auto end = high_resolution_clock::now();
	nanoseconds time = end - start;
	int size = newMontageIndex.size();
	string str = "Need to compile " + to_string(size) + " montages: " + to_string(static_cast<double>(time.count())/1000/1000) + " ms";
	if (size > 0)
	{
		logToFileAndConsole(str);
	}
	else
	{
		logToFile(str);
	}
#endif

	releaseOutputBuffer();

	unsigned int outputBlockSize = blockSize*trackCount; // Code-duplicity.
	//outputBlockSize *= PROGRAM_OPTIONS["eventRenderMode"].as<int>();

	cl_int err;

	if (glSharing)
	{
		gl()->glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		gl()->glBufferData(GL_ARRAY_BUFFER, outputBlockSize*sizeof(float), nullptr, GL_STATIC_DRAW);

		cl_mem_flags flags = CL_MEM_READ_WRITE;
	#ifdef NDEBUG
		flags = CL_MEM_WRITE_ONLY;
	#if CL_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
	#endif
	#endif

		processorOutputBuffer = clCreateFromGLBuffer(context->getCLContext(), flags, glBuffer, &err);
		checkClErrorCode(err, "clCreateFromGLBuffer()");
	}
	else
	{
		cl_mem_flags flags = CL_MEM_READ_WRITE;
	#ifdef NDEBUG
	#if CL_1_2
		flags |= CL_MEM_HOST_READ_ONLY;
	#endif
	#endif

		processorOutputBuffer = clCreateBuffer(context->getCLContext(), flags, outputBlockSize*sizeof(float), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		assert(!processorOutputBufferTmp && "Make sure the buffer does't leak.");
		processorOutputBufferTmp = new float[outputBlockSize];
	}

	gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SignalProcessor::deleteMontage()
{
	montage.clear();
}
