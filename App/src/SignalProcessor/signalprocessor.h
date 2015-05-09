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
#include <cassert>

/**
 * @brief A class used for retrieving the processed signal blocks.
 *
 * This is the main class in the SignalProcessor package. As such it is the only
 * one directly accessed from the rest of the code.
 *
 * Before an object of this class is constructed or used an OpenGL context must be set current.
 * There is example usage outside of a method that guarantees this:
 * \code{.cpp}
 * makeCurrent();
 * signalProcessor->setUpdateMontageFlag();
 * doneCurrent();
 * \endcode
 *
 */
class SignalProcessor : public OpenGLInterface
{
public:
	SignalProcessor();
	~SignalProcessor();

	/**
	 * @brief Returns the number of samples in one channel of the result signal block.
	 */
	unsigned int getBlockSize() const
	{
		return blockSize;
	}

	/**
	 * @brief Returns the number of channels of the result signal block.
	 */
	int getTrackCount() const
	{
		return trackCount;
	}

	/**
	 * @brief Updates the currently used filter according to the values set in the InfoTable.
	 *
	 * The filter is updated immediately.
	 */
	void updateFilter();

	/**
	 * @brief The montage will be updated next time it is needed.
	 *
	 * The update is lazy -- consecutive calls will do nothing.
	 * The reason for this is that needles repeated compilation of the montage code
	 * caused significant slowdown.
	 */
	void setUpdateMontageFlag();

//	unsigned int getCapacity() const
//	{
//		if (ready() == false)
//		{
//			return -1;
//		}
//		return cache->getCapacity();
//	}

	/**
	 * @brief Returns any block from indexSet ready to be used for rendering.
	 * @param indexSet Requested block indexes.
	 * @return An object wrapping the block index (and other info) and the vertex arrays for accessing the data.
	 *
	 * In this method the synchronization between OpenGL and OpenCL is performed
	 * through glFinish() and clFinish().
	 *
	 * Montage is updated if needed.
	 *
	 * If onlineFilter is set in Options filtration is performed here also.
	 *
	 * When called ready() should be true.
	 */
	SignalBlock getAnyBlock(const std::set<int>& indexSet);

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(DataFile* file);

	/**
	 * @brief Returns true if this object is ready for full operation.
	 */
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
	bool updateMontageFlag = false;

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
			checkClErrorCode(err, "clReleaseMemObject()");

			processorOutputBuffer = nullptr;
		}
	}

	/**
	 * @brief This method actually (unlike setUpdateMontageFlag()) updates the MontageProcessor object.
	 */
	void updateMontage();
};

#endif // SIGNALPROCESSOR_H
