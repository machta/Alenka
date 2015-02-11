#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "../openglinterface.h"

#include "signalblock.h"
#include "../DataFile/datafile.h"
#include "../options.h"

#include <inttypes.h>
#include <set>

class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor(DataFile* file, uint memory = 1024*1024*1024, double bufferRatio = 1);
	~SignalProcessor();

	int64_t getBlockSize()
	{
		return PROGRAM_OPTIONS->get("blockSize").as<int>();
	}

	// ..

	SignalBlock getAnyBlock(const std::set<unsigned int>& index);

private:
	DataFile* dataFile;
	GLuint buffer;
	float* tmpBuffer;
};

#endif // SIGNALPROCESSOR_H
