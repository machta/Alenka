#include "signalprocessor.h"

#include "../DataModel/opendatafile.h"
#include <AlenkaFile/datafile.h>
#include "../myapplication.h"
#include "signalblock.h"
#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/filter.h>
#include <AlenkaSignal/filterprocessor.h>
#include <AlenkaSignal/montageprocessor.h>
#include "../options.h"
#include "../error.h"

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

SignalProcessor::SignalProcessor(unsigned int nBlock, unsigned int parallelQueues, int montageCopyCount, OpenDataFile* file, AlenkaSignal::OpenCLContext* context)
	: nBlock(nBlock), parallelQueues(parallelQueues), montageCopyCount(montageCopyCount), file(file), context(context)
{
	fileChannels = file->file->getChannelCount();
	cl_int err;

	initializeOpenGLInterface();

	for (unsigned int i = 0; i < parallelQueues; ++i)
	{
		commandQueues.push_back(clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err));
		checkClErrorCode(err, "clCreateCommandQueue()");

		cl_mem_flags flags = CL_MEM_READ_WRITE;
		inBuffers.push_back(clCreateBuffer(context->getCLContext(), flags, (nBlock + 2)*fileChannels*sizeof(float), nullptr, &err));
		checkClErrorCode(err, "clCreateBuffer()");

#ifdef NDEBUG
#if CL_1_2
		flags |= CL_MEM_HOST_NO_ACCESS;
#endif
#endif
		outBuffers.push_back(clCreateBuffer(context->getCLContext(), flags, (nBlock + 2)*fileChannels*sizeof(float), nullptr, &err));
		checkClErrorCode(err, "clCreateBuffer()");
	}

	fileBuffer = new float[nBlock*fileChannels];
	filterProcessor = new AlenkaSignal::FilterProcessor<float>(nBlock, fileChannels, context);

	QFile headerFile(":/montageHeader.cl"); // TODO: Consolidate the 4 copies of this into one instance.
	headerFile.open(QIODevice::ReadOnly);
	header = headerFile.readAll().toStdString();

	updateFilter();
	setUpdateMontageFlag();
}

SignalProcessor::~SignalProcessor()
{
	cl_int err;

	deleteMontage();

	for (unsigned int i = 0; i < parallelQueues; ++i)
	{
		err = clReleaseCommandQueue(commandQueues[i]);
		checkClErrorCode(err, "clReleaseCommandQueue()");

		err = clReleaseMemObject(inBuffers[i]);
		checkClErrorCode(err, "clReleaseMemObject()");

		err = clReleaseMemObject(outBuffers[i]);
		checkClErrorCode(err, "clReleaseMemObject()");
	}

	delete[] fileBuffer;
	delete filterProcessor;
	delete montageProcessor;
}

void SignalProcessor::updateFilter()
{
	using namespace std;

	if (!file)
		return;

	M = file->file->getSamplingFrequency() + 1;
	AlenkaSignal::Filter<float> filter(M, file->file->getSamplingFrequency()); // TODO: Possibly could save this object so that it won't be created from scratch every time.

	filter.lowpass(true);
	filter.setLowpass(OpenDataFile::infoTable.getLowpassFrequency());

	filter.highpass(true);
	filter.setHighpass(OpenDataFile::infoTable.getHighpassFrequency());

	filter.notch(OpenDataFile::infoTable.getNotch());
	filter.setNotch(PROGRAM_OPTIONS["notchFrequency"].as<double>());

	auto samples = filter.computeSamples();
	if (OpenDataFile::infoTable.getFrequencyMultipliersOn())
		multiplySamples(&samples);

	filterProcessor->changeSampleFilter(M, samples);
	filterProcessor->applyWindow(OpenDataFile::infoTable.getFilterWindow());

	nDelay = filterProcessor->delaySamples();
	nMontage = nBlock - M + 1;
	nSamples = nMontage - 2;

	OpenDataFile::infoTable.setFilterCoefficients(filterProcessor->getCoefficients());
}

void SignalProcessor::setUpdateMontageFlag()
{
	if (file)
	{
		trackCount = 0;

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

void SignalProcessor::process(const vector<int>& index, const vector<cl_mem>& buffers)
{
	assert(ready());
	assert(0 < index.size());
	assert(static_cast<unsigned int>(index.size()) <= parallelQueues);
	assert(index.size() == buffers.size());

	if (updateMontageFlag)
	{
		updateMontageFlag = false;
		updateMontage();
	}

	cl_int err;
	const unsigned int iters = min<unsigned int>(parallelQueues, index.size());

	for (unsigned int i = 0; i < iters; ++i)
	{
		auto fromTo = blockIndexToSampleRange(index[i], nSamples);
		fromTo.first += -M + nDelay;
		fromTo.second += nDelay + 1;
		assert(fromTo.second - fromTo.first + 1 == nBlock);

		file->file->readSignal(fileBuffer, fromTo.first, fromTo.second);
		printBuffer("after_readSignal.txt", fileBuffer, nBlock*fileChannels);

		size_t origin[] = {0, 0, 0};
		size_t rowLen = nBlock*sizeof(float);
		size_t region[] = {rowLen, fileChannels, 1};

		err = clEnqueueWriteBufferRect(commandQueues[i], inBuffers[i], CL_TRUE, origin, origin, region, rowLen + 2*sizeof(float), 0, 0, 0, fileBuffer, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueWriteBufferRect()");

		printBuffer("before_filter.txt", inBuffers[i], commandQueues[i]);
		filterProcessor->process(inBuffers[i], outBuffers[i], commandQueues[i]);
		printBuffer("after_filter.txt", outBuffers[i], commandQueues[i]);
	}

	gl()->glFinish(); // Could be replaced by a fence.

	for (unsigned int i = 0; i < iters; ++i)
	{
		err = clEnqueueAcquireGLObjects(commandQueues[i], 1, &buffers[i], 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueAcquireGLObjects()");

		montageProcessor->process(montage, outBuffers[i], buffers[i], commandQueues[i], M - 1);
		printBuffer("after_montage.txt", buffers[i], commandQueues[i]);
	}

	for (unsigned int i = 0; i < iters; ++i)
	{
		err = clFinish(commandQueues[i]); // Why is this here twice.
		checkClErrorCode(err, "clFinish()");

		err = clEnqueueReleaseGLObjects(commandQueues[i], 1, &buffers[i], 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReleaseGLObjects()");

		err = clFinish(commandQueues[i]);
		checkClErrorCode(err, "clFinish()");
	}
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

void SignalProcessor::updateMontage()
{
	assert(ready());

	delete montageProcessor;
	montageProcessor = new AlenkaSignal::MontageProcessor<float>(nBlock + 2, nMontage, fileChannels, montageCopyCount);

	deleteMontage();
	vector<string> montageCode;

	for (int i = 0; i < getTrackTable(file)->rowCount(); i++)
	{
		Track t = getTrackTable(file)->row(i);
		if (!t.hidden)
			montageCode.push_back(t.code);
	}

	montage = makeMontage(montageCode, context, file->kernelCache, header);
}

void SignalProcessor::deleteMontage()
{
	for (auto e : montage)
		delete e;
	montage.clear();
}
