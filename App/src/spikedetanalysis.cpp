#include "spikedetanalysis.h"

#include "DataFile/datafile.h"
#include <openclcontext.h>
#include <montage.h>
#include <montageprocessor.h>

using namespace std;

namespace
{

template<class T>
class Loader : public SpikedetDataLoader<T>
{
	const int BLOCK_LENGTH = 8*1024/*512*/;

	DataFile* file;
	const std::vector<Montage<T>*>& montage;

	MontageProcessor<T>* processor;
	vector<T> tmpData;
	int inChannels, outChannels;
	cl_command_queue queue = nullptr;
	cl_mem inBuffer = nullptr, outBuffer = nullptr;

public:
	Loader(DataFile* file, const std::vector<Montage<T>*>& montage, OpenCLContext* context) :
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
			size_t inRegion[] = {rowLen, inChannels, 1};

			err = clEnqueueWriteBufferRect(queue, inBuffer, CL_FALSE, origin, origin, inRegion, BLOCK_LENGTH*sizeof(T), 0, 0, 0, tmpData.data(), 0, nullptr, nullptr); // SEGFAULT
			checkClErrorCode(err, "clEnqueueWriteBufferRect()");

			processor->process(montage, inBuffer, outBuffer, queue);
			//OpenCLContext::printBuffer("after_process.txt", outBuffer, queue);

			size_t outRegion[] = {rowLen, outChannels, 1};
			size_t dataOrigin[] = {(sample - firstSample)*sizeof(T), 0, 0};

			err =  clEnqueueReadBufferRect(queue, outBuffer, CL_FALSE, origin, dataOrigin, outRegion, BLOCK_LENGTH*sizeof(T), 0,
											(lastSample - firstSample + 1)*sizeof(T), 0, data, 0, nullptr, nullptr);
			checkClErrorCode(err, "clEnqueueReadBufferRect()");
		}
	}
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

void SpikedetAnalysis::runAnalysis(DataFile* file, const std::vector<Montage<float>*>& montage)
{
	assert(file != nullptr);

	Spikedet<float> spikedet(file->getSamplingFrequency(), montage.size(), settings, context);
	Loader<float> loader(file, montage, context);

	delete output;
	output = new CDetectorOutput;

	delete discharges;
	discharges = new CDischarges(file->getChannelCount());

	spikedet.runAnalysis(&loader, output, discharges);
}
