#include "signalprocessor.h"

#include "../DataModel/opendatafile.h"
#include <AlenkaFile/datafile.h>
#include "../myapplication.h"
#include "signalblock.h"
#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filter.h>
#include <AlenkaSignal/filterprocessor.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaSignal/montageprocessor.h>

#include <QFile>

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <sstream>

using namespace std;
using namespace AlenkaFile;

namespace
{

const AbstractTrackTable* getTrackTable(OpenDataFile* file)
{
	return file->dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
}

void multiplySamples(vector<float>* samples)
{
	// Assume input is already sorted.
	vector<pair<double, double>> input = OpenDataFile::infoTable.getFrequencyMultipliers();
	if (input.empty())
		return;

	int inputSize = static_cast<int>(input.size());
	int samplesSize = static_cast<int>(samples->size());
	input.push_back(make_pair(samplesSize, input.back().second)); // End of vector guard.

	vector<float> multipliers(samplesSize, 1);

	for (int i = 0; i < inputSize; ++i)
	{
		double multi = input[i].second;
		int f = round(input[i].first);
		double nextF = input[i + 1].first;

		if (samplesSize < f || samplesSize < nextF)
			continue;

		for (; f < nextF; ++f)
			multipliers[f] = multi;
	}

	for (int i = 0; i < samplesSize; ++i)
		(*samples)[i] *= multipliers[i];
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

	QFile headerFile(":/montageHeader.cl"); // TODO: Consolidate the 4 copies of this into one instance.
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
}

void SignalProcessor::updateFilter()
{
	using namespace std;

	if (!file)
		return;

	int M = file->file->getSamplingFrequency()/* + 1*/;
	AlenkaSignal::Filter<float> filter(M, file->file->getSamplingFrequency()); // TODO: Possibly could save this object so that it won't be created from scratch every time.

	filter.lowpass(true);
	filter.setLowpass(OpenDataFile::infoTable.getLowpassFrequency());

	filter.highpass(true);
	filter.setHighpass(OpenDataFile::infoTable.getHighpassFrequency());

	filter.notch(OpenDataFile::infoTable.getNotch());
	filter.setNotch(50);

	auto samples = filter.computeSamples();
	if (OpenDataFile::infoTable.getFrequencyMultipliersOn())
		multiplySamples(&samples);

	filterProcessor->changeSampleFilter(M, samples);
	filterProcessor->applyWindow(OpenDataFile::infoTable.getFilterWindow());

	OpenDataFile::infoTable.setFilterCoefficients(filterProcessor->getCoefficients());

	if (onlineFilter == false)
		cache->clear();
}

void SignalProcessor::setUpdateMontageFlag()
{
	if (file)
	{
		trackCount = 0; // TODO: Investigate what this variable is for.

		if (0 < file->dataModel->montageTable()->rowCount())
		{
			for (int i = 0; i < getTrackTable(file)->rowCount(); ++i)
			{
				if (getTrackTable(file)->row(i).hidden == false)
					++trackCount;
			}
		}

		if (trackCount > 0)
			updateMontageFlag = true;
	}
}

SignalBlock SignalProcessor::getAnyBlock(const set<int>& indexSet)
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

void SignalProcessor::changeFile(OpenDataFile* file)
{
	destroyFileRelated();

	this->file = file;

	if (file)
	{
		int M = file->file->getSamplingFrequency();
		int offset = M;
		int delay = M/2 - 1;
		blockSize = PROGRAM_OPTIONS["blockSize"].as<unsigned int>() - offset;
		unsigned int tmpBlockSize = (blockSize + offset)*file->file->getChannelCount();

		// Construct the filter and montage processors.
		filterProcessor = new AlenkaSignal::FilterProcessor<float>(blockSize + offset, file->file->getChannelCount(), context);

		montageProcessor = new AlenkaSignal::MontageProcessor<float>(offset, blockSize, file->file->getChannelCount());

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

vector<AlenkaSignal::Montage<float>*> SignalProcessor::makeMontage(const vector<string>& montageCode, AlenkaSignal::OpenCLContext* context, KernelCache* kernelCache, const string& header)
{
	using namespace chrono;
#ifndef NDEBUG
	auto start = high_resolution_clock::now(); // TODO: Remove this after the compilation time issue is solved.
	int needToCompile = 0;
#endif

	vector<AlenkaSignal::Montage<float>*> montage;

	for (unsigned int i = 0; i < montageCode.size(); i++)
	{
		AlenkaSignal::Montage<float>* m;
		QString code = QString::fromStdString(simplifyMontage(montageCode[i]));

		auto ptr = kernelCache ? kernelCache->find(code) : nullptr;

		if (ptr)
		{
			assert(0 < ptr->size());
			m = new AlenkaSignal::Montage<float>(ptr, context);
		}
		else
		{
#ifndef NDEBUG
			++needToCompile;
#endif
			m = new AlenkaSignal::Montage<float>(code.toStdString(), context, header);

			if (kernelCache)
			{
				auto binary = m->getBinary();
				if (binary->size() > 0)
					kernelCache->insert(code, binary);
			}
		}

		montage.push_back(m);
	}

#ifndef NDEBUG
	auto end = high_resolution_clock::now();
	nanoseconds time = end - start;
	string str = "Need to compile " + to_string(needToCompile) + " montages: " + to_string(static_cast<double>(time.count())/1000/1000) + " ms";
	if (needToCompile > 0)
	{
		logToFileAndConsole(str);
	}
	else
	{
		logToFileAndConsole(str)
		//logToFile(str);
	}
#endif

	return montage;
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

string SignalProcessor::indexSetToString(const set<int>& indexSet)
{
	stringstream ss;

	for (const auto& e : indexSet)
	{
		if (e != *indexSet.begin())
			ss << ", ";
		ss << e;
	}

	return ss.str();
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
	assert(ready());
	deleteMontage();

	vector<string> montageCode;
	for (int i = 0; i < getTrackTable(file)->rowCount(); i++)
	{
		Track t = getTrackTable(file)->row(i);
		if (!t.hidden)
			montageCode.push_back(t.code);
	}

	montage = makeMontage(montageCode, context, file->kernelCache, header);

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
	for (auto e : montage)
		delete e;
	montage.clear();
}
