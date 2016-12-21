#include "spikedetanalysis.h"

#include "DataFile/datafile.h"

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaSignal/montageprocessor.h>

#include <QProgressDialog>

#include <thread>

using namespace std;
using namespace AlenkaSignal;

namespace
{

/*template<class T>
class Loader : public SpikedetDataLoader<T>
{
	DataFile* file;
	vector<T> tmpData;

public:
	Loader(DataFile* file) : file(file)
	{
	}
	virtual ~Loader() override
	{}

	virtual void readSignal(T* data, int64_t firstSample, int64_t lastSample) override
	{
		int64_t len = (lastSample - firstSample + 1)*channelCount();
		tmpData.resize(len);

		file->readData(&tmpData, firstSample, lastSample);

		for (int64_t i = 0; i < len; i++) // TODO: remove this needless copying after the new File lib is integrated
			data[i] = tmpData[i];
	}
	virtual int64_t sampleCount() override
	{
		return file->getSamplesRecorded();
	}
	virtual int channelCount() override
	{
		return file->getChannelCount();
	}
};*/

template<class T>
class Loader : public SpikedetDataLoader<T>
{
	const int BLOCK_LENGTH = 8*1024/*512*/;

	DataFile* file;
	const vector<Montage<T>*>& montage;

	MontageProcessor<T>* processor;
	vector<T> tmpData;
	int inChannels, outChannels;
	cl_command_queue queue = nullptr;
	cl_mem inBuffer = nullptr, outBuffer = nullptr;

public:
	Loader(DataFile* file, const vector<Montage<T>*>& montage, OpenCLContext* context) :
		file(file), montage(montage), inChannels(file->getChannelCount()), outChannels(montage.size())
	{
		processor = new MontageProcessor<T>(0, BLOCK_LENGTH, inChannels);

		cl_int err;
		cl_mem_flags flags = CL_MEM_READ_WRITE;

		queue = clCreateCommandQueue(context->getCLContext(), context->getCLDevice(), 0, &err);
		checkClErrorCode(err, "clCreateCommandQueue");

		inBuffer = clCreateBuffer(context->getCLContext(), flags, BLOCK_LENGTH*inChannels*sizeof(T), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		outBuffer = clCreateBuffer(context->getCLContext(), flags, BLOCK_LENGTH*outChannels*sizeof(T), nullptr, &err);
		checkClErrorCode(err, "clCreateBuffer");

		tmpData.resize(BLOCK_LENGTH*inChannels);
	}
	virtual ~Loader() override
	{
		delete processor;

		cl_int err;

		if (queue != nullptr)
		{
			err = clReleaseCommandQueue(queue);
			checkClErrorCode(err, "clReleaseCommandQueue()");
		}

		if (inBuffer != nullptr)
		{
			err = clReleaseMemObject(inBuffer);
			checkClErrorCode(err, "clReleaseMemObject()");
		}
		if (outBuffer != nullptr)
		{
			err = clReleaseMemObject(outBuffer);
			checkClErrorCode(err, "clReleaseMemObject()");
		}
	}

	virtual void readSignal(T* data, int64_t firstSample, int64_t lastSample) override
	{
		cl_int err;

		for (int64_t sample = firstSample; sample <= lastSample; sample += BLOCK_LENGTH)
		{
			//OpenCLContext::printBuffer("spikedet_loader.txt", data, (lastSample - firstSample + 1)*outChannels);
			int len = min<int>(BLOCK_LENGTH, lastSample - sample + 1);
			assert(len >= 1);

			file->readData(&tmpData, sample, sample + len - 1);

			size_t origin[] = {0, 0, 0};
			size_t rowLen = len*sizeof(T);
			size_t inRegion[] = {rowLen, static_cast<size_t>(inChannels), 1};

			err = clEnqueueWriteBufferRect(queue, inBuffer, CL_TRUE, origin, origin, inRegion, BLOCK_LENGTH*sizeof(T), 0, 0, 0, tmpData.data(), 0, nullptr, nullptr); // SEGFAULT
			checkClErrorCode(err, "clEnqueueWriteBufferRect()");

			processor->process(montage, inBuffer, outBuffer, queue);
			//OpenCLContext::printBuffer("after_process.txt", outBuffer, queue);

			size_t outRegion[] = {rowLen, static_cast<size_t>(outChannels), 1};
			size_t dataOrigin[] = {(sample - firstSample)*sizeof(T), 0, 0};

			err =  clEnqueueReadBufferRect(queue, outBuffer, CL_TRUE, origin, dataOrigin, outRegion, BLOCK_LENGTH*sizeof(T), 0,
											(lastSample - firstSample + 1)*sizeof(T), 0, data, 0, nullptr, nullptr);
			checkClErrorCode(err, "clEnqueueReadBufferRect()");
		}
		//OpenCLContext::printBuffer("spikedet_loader_done.txt", data, (lastSample - firstSample + 1)*outChannels);
	}

	/*virtual void readSignal(T* data, int64_t firstSample, int64_t lastSample) override
	{ // This version reads always the whole block from the file.
		cl_int err;

		for (int64_t sample = firstSample; sample <= lastSample; sample += BLOCK_LENGTH)
		{
			//OpenCLContext::printBuffer("spikedet_loader.txt", data, (lastSample - firstSample + 1)*outChannels);
			int len = min<int>(BLOCK_LENGTH, lastSample - sample + 1);
			assert(len >= 1);

			file->readData(&tmpData, sample, sample + BLOCK_LENGTH - 1);

			err = clEnqueueWriteBuffer(queue, inBuffer, CL_TRUE, 0, BLOCK_LENGTH*inChannels*sizeof(T), tmpData.data(), 0, nullptr, nullptr);
			checkClErrorCode(err, "clEnqueueWriteBuffer()");

			processor->process(montage, inBuffer, outBuffer, queue);
			//OpenCLContext::printBuffer("after_process.txt", outBuffer, queue);

			size_t origin[] = {0, 0, 0};
			size_t dataOrigin[] = {(sample - firstSample)*sizeof(T), 0, 0};
			size_t rowLen = len*sizeof(T);
			size_t region[] = {rowLen, static_cast<size_t>(outChannels), 1};

			err = clEnqueueReadBufferRect(queue, outBuffer, CL_TRUE, origin, dataOrigin, region, BLOCK_LENGTH*sizeof(T), 0,
										   (lastSample - firstSample + 1)*sizeof(T), 0, data, 0, nullptr, nullptr);
			checkClErrorCode(err, "clEnqueueReadBufferRect()");
		}
		//OpenCLContext::printBuffer("spikedet_loader_done.txt", data, (lastSample - firstSample + 1)*outChannels);
	}*/

	virtual int64_t sampleCount() override
	{
		return file->getSamplesRecorded();
	}
	virtual int channelCount() override
	{
		return outChannels;
	}
};

} // namespace

void SpikedetAnalysis::runAnalysis(DataFile* file, const vector<Montage<float>*>& montage, QProgressDialog* progress)
{
	assert(file != nullptr);

	Spikedet<float> spikedet(file->getSamplingFrequency(), montage.size(), settings, context);
	Loader<float> loader(file, montage, context);

	delete output;
	output = new CDetectorOutput;

	delete discharges;
	discharges = new CDischarges(file->getChannelCount());

	thread t([&] () { spikedet.runAnalysis(&loader, output, discharges); });

	while (1)
	{
		this_thread::sleep_for(std::chrono::milliseconds(10));

		int percentage = spikedet.progressPercentage();
		progress->setValue(percentage);

		if (progress->wasCanceled())
		{
			spikedet.cancel();
			break;
		}

		if (percentage == 100)
			break;
	}

	t.join();
	progress->setValue(100);
}
