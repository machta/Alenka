/**
 * @brief This file defines some global auxiliary functions and macros (mostly for error handling).
 *
 * Convenience functions (further simplified by macros) for handling
 * of error codes and for accessing the log are defined here.
 *
 * The message_ parameter of these macros accepts expressions like:
 *
 * * "a string"
 * * "the value is" << integerVar
 * * "a string" << std::endl << "some more on a new line"
 *
 * @file
 */

#ifndef ERROR_H
#define ERROR_H

#include "options.h"

#include <CL/cl_gl.h>
#include <clFFT.h>

#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <ctime>
#include <string>
#include <sstream>

#include <QFile>

/**
 * @brief Returns error code converted to a suitable form for printing.
 */
template <typename T>
inline std::string errorCodeToString(T val)
{
	using namespace std;

	stringstream ss;

	ss << dec << val << "(0x" << hex << val << dec << ")";

	return ss.str();
}

/// @cond
namespace
{
template <typename T>
void CEC(T val, T expected, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Unexpected error code: ";
	ss << errorCodeToString(val);
	ss << ", required ";
	ss << errorCodeToString(expected);
	ss << ". ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

template <typename T>
void CNEC(T val, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Error code returned ";
	ss << errorCodeToString(val);
	ss << ". ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

#define CASE(a_) case a_: return #a_

std::string clErrorCodeToString(cl_int code)
{
	using namespace std;

	switch (code)
	{
		CASE(CL_SUCCESS);
		CASE(CL_DEVICE_NOT_FOUND);
		CASE(CL_DEVICE_NOT_AVAILABLE);
		CASE(CL_COMPILER_NOT_AVAILABLE);
		CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
		CASE(CL_OUT_OF_RESOURCES);
		CASE(CL_OUT_OF_HOST_MEMORY);
		CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
		CASE(CL_MEM_COPY_OVERLAP);
		CASE(CL_IMAGE_FORMAT_MISMATCH);
		CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
		CASE(CL_BUILD_PROGRAM_FAILURE);
		CASE(CL_MAP_FAILURE);
		CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET);
		CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
#if CL_1_2
		CASE(CL_COMPILE_PROGRAM_FAILURE);
		CASE(CL_LINKER_NOT_AVAILABLE);
		CASE(CL_LINK_PROGRAM_FAILURE);
		CASE(CL_DEVICE_PARTITION_FAILED);
		CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
#endif
		CASE(CL_INVALID_VALUE);
		CASE(CL_INVALID_DEVICE_TYPE);
		CASE(CL_INVALID_PLATFORM);
		CASE(CL_INVALID_DEVICE);
		CASE(CL_INVALID_CONTEXT);
		CASE(CL_INVALID_QUEUE_PROPERTIES);
		CASE(CL_INVALID_COMMAND_QUEUE);
		CASE(CL_INVALID_HOST_PTR);
		CASE(CL_INVALID_MEM_OBJECT);
		CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
		CASE(CL_INVALID_IMAGE_SIZE);
		CASE(CL_INVALID_SAMPLER);
		CASE(CL_INVALID_BINARY);
		CASE(CL_INVALID_BUILD_OPTIONS);
		CASE(CL_INVALID_PROGRAM);
		CASE(CL_INVALID_PROGRAM_EXECUTABLE);
		CASE(CL_INVALID_KERNEL_NAME);
		CASE(CL_INVALID_KERNEL_DEFINITION);
		CASE(CL_INVALID_KERNEL);
		CASE(CL_INVALID_ARG_INDEX);
		CASE(CL_INVALID_ARG_VALUE);
		CASE(CL_INVALID_ARG_SIZE);
		CASE(CL_INVALID_KERNEL_ARGS);
		CASE(CL_INVALID_WORK_DIMENSION);
		CASE(CL_INVALID_WORK_GROUP_SIZE);
		CASE(CL_INVALID_WORK_ITEM_SIZE);
		CASE(CL_INVALID_GLOBAL_OFFSET);
		CASE(CL_INVALID_EVENT_WAIT_LIST);
		CASE(CL_INVALID_EVENT);
		CASE(CL_INVALID_OPERATION);
		CASE(CL_INVALID_GL_OBJECT);
		CASE(CL_INVALID_BUFFER_SIZE);
		CASE(CL_INVALID_MIP_LEVEL);
		CASE(CL_INVALID_GLOBAL_WORK_SIZE);
		CASE(CL_INVALID_PROPERTY);
#if CL_1_2
		CASE(CL_INVALID_IMAGE_DESCRIPTOR);
		CASE(CL_INVALID_COMPILER_OPTIONS);
		CASE(CL_INVALID_LINKER_OPTIONS);
		CASE(CL_INVALID_DEVICE_PARTITION_COUNT);
#endif
	}

	return "unknown code " + errorCodeToString(code);
}

std::string clfftErrorCodeToString(clfftStatus code)
{
	switch (code)
	{
		CASE(CLFFT_BUGCHECK);
		CASE(CLFFT_NOTIMPLEMENTED);
		CASE(CLFFT_TRANSPOSED_NOTIMPLEMENTED);
		CASE(CLFFT_FILE_NOT_FOUND);
		CASE(CLFFT_FILE_CREATE_FAILURE);
		CASE(CLFFT_VERSION_MISMATCH);
		CASE(CLFFT_INVALID_PLAN);
		CASE(CLFFT_DEVICE_NO_DOUBLE);
		CASE(CLFFT_DEVICE_MISMATCH);
		CASE(CLFFT_ENDSTATUS);
	default:
		return clErrorCodeToString(code);
	}
}

#undef CASE

void CCEC(cl_int val, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Unexpected error code: ";
	ss << clErrorCodeToString(val);
	ss << ", required CL_SUCCESS. ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

void CFCEC(clfftStatus val, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Unexpected error code: " << clfftErrorCodeToString(val);
	ss << ", required CLFFT_SUCCESS. ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

const char* getTimeString(std::time_t t)
{
	using namespace std;

	string tmp = asctime(localtime(&t));
	if (tmp.back() == '\n')
	{
		tmp.pop_back();
	}
	return tmp.c_str();
}
}
/// @endcond

/**
 * @brief Compared the returned error code with the success value.
 * @param val_ The error code.
 * @param expected_ The expected error code.
 */
#define checkErrorCode(val_, expected_, message_) if((val_) != (expected_)) { std::stringstream ss; ss << message_; CEC(val_, expected_, ss.str(), __FILE__, __LINE__); }

/**
 * @brief Compared the returned error code with the failure value.
 * @param val_The error code.
 * @param notExpected_ The error code you are trying to avoid.
 */
#define checkNotErrorCode(val_, notExpected_, message_) if((val_) == (notExpected_)) { std::stringstream ss; ss << message_; CNEC(val_, ss.str(), __FILE__, __LINE__); }

/**
 * @brief Simplified error code test for OpenCL functions.
 * @param val_ The error code.
 */
#define checkClErrorCode(val_, message_) if((val_) != CL_SUCCESS) { std::stringstream ss; ss << message_; CCEC(val_, ss.str(), __FILE__, __LINE__); }

/**
 * @brief Simplified error code test for clFFT functions
 * @param val_ The error code.
 */
#define checkClfftErrorCode(val_, message_) if((val_) != CLFFT_SUCCESS) { std::stringstream ss; ss << message_; CFCEC(val_, ss.str(), __FILE__, __LINE__); }

/**
 * @brief Same as std::fread() but when an error is detected an exception is thrown.
 */
inline std::size_t freadChecked(void* data, std::size_t size, std::size_t n, FILE* file)
{
	using namespace std;

	size_t elementsRead = fread(data, size, n, file);
	if (elementsRead != n)
	{
		const char* message;

		if (feof(file))
		{
			message = "EOF reached prematurely.";
		}
		else
		{
			assert(ferror(file));
			message = "Error while reading data from file.";
		}
		throw runtime_error(message);
	}
	return elementsRead;
}

/**
 * @brief Same as std::fwrite() but when an error is detected an exception is thrown.
 */
inline std::size_t fwriteChecked(void* data, std::size_t size, std::size_t n, FILE* file)
{
	using namespace std;

	size_t elementsWritten = fwrite(data, size, n, file);
	if (elementsWritten != n)
	{
		throw runtime_error("Error while reading data from file.");
	}
	return elementsWritten;
}

/**
 * @brief Reads the whole content of the text file and returns it as a string.
 */
inline std::string readWholeTextFile(FILE* file)
{
	using namespace std;

	int err = fseek(file, 0, SEEK_END);
	checkErrorCode(err, 0, "fseek() in readWholeTextFile()");

	size_t size = ftell(file);
	char* tmp = new char[size + 1];
	tmp[size] = 0;

	rewind(file);
	freadChecked(tmp, sizeof(char), size, file);

	string str(tmp);
	delete[] tmp;
	return str;
}

inline std::string readWholeTextFile(QFile* file)
{
	std::string str;

	char c;
	while (file->getChar(&c))
		str.push_back(c);

	return str;
}

/**
 * @brief Prints the content of the data array to a file in a human-readable format.
 *
 * Used for debugging purposes.
 */
inline void printBuffer(FILE* file, float* data, int n)
{
#ifndef NDEBUG
	if (PROGRAM_OPTIONS.isSet("printBuffers"))
	{
		for (int i = 0; i < n; ++i)
		{
			fprintf(file, "%f\n", data[i]);
		}
	}
#else
	(void)file;
	(void)data;
	(void)n;
#endif
}

/**
 * @brief Prints the content of the OpenCL buffer to a file in a human-readable format.
 *
 * Used for debugging purposes.
 */
inline void printBuffer(FILE* file, cl_mem buffer, cl_command_queue queue)
{
#ifndef NDEBUG
	if (PROGRAM_OPTIONS.isSet("printBuffers"))
	{
		cl_int err;

		size_t size;
		err = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size_t), &size, nullptr);
		checkClErrorCode(err, "clGetMemObjectInfo");

		float* tmp = new float[size/sizeof(float)];

		err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, tmp, 0, nullptr, nullptr);
		checkClErrorCode(err, "clEnqueueReadBuffer");

		printBuffer(file, tmp, size/sizeof(float));

		delete[] tmp;
	}
#else
	(void)file;
	(void)buffer;
	(void)queue;
#endif
}

/**
 * @brief Prints the content of the data array to a new file with filePath in a human-readable format.
 *
 * Used for debugging purposes.
 */
inline void printBuffer(const std::string& filePath, float* data, int n)
{
#ifndef NDEBUG
	using namespace std;

	if (PROGRAM_OPTIONS.isSet("printBuffers"))
	{
		string path = PROGRAM_OPTIONS["printBuffersFolder"].as<string>() + "/" + filePath;

		FILE* file = fopen(path.c_str(), "w");
		checkNotErrorCode(file, nullptr, "File '" << path << "' could not be open for writing.");

		printBuffer(file, data, n);

		int err = fclose(file);
		checkErrorCode(err, 0, "fclose()");
	}
#else
	(void)filePath;
	(void)data;
	(void)n;
#endif
}

/**
 * @brief Prints the content of the OpenCL buffer to a new file with filePath in a human-readable format.
 *
 * Used for debugging purposes.
 */
inline void printBuffer(const std::string& filePath, cl_mem buffer, cl_command_queue queue)
{
#ifndef NDEBUG
	using namespace std;

	if (PROGRAM_OPTIONS.isSet("printBuffers"))
	{
		string path = PROGRAM_OPTIONS["printBuffersFolder"].as<string>() + "/" + filePath;

		FILE* file = fopen(path.c_str(), "w");
		checkNotErrorCode(file, nullptr, "File '" << path << "' could not be open for writing.");

		printBuffer(file, buffer, queue);

		int err = fclose(file);
		checkErrorCode(err, 0, "fclose()");
	}
#else
	(void)filePath;
	(void)buffer;
	(void)queue;
#endif
}

/**
 * @brief Log file object.
 */
extern std::ofstream LOG_FILE;

/**
 * @brief Log file mutex.
 *
 * This ensures that the the log is not broken due to race conditions.
 */
extern std::mutex LOG_FILE_MUTEX;

/**
 * @brief Logs a message to the stream.
 *
 * The format of the log entries (for the log file and stderr) is defined here.
 */
#define logToStream(message_, stream_) stream_ <<  message_ << " [" << "in T" << std::this_thread::get_id() << " from " << __FILE__ << ":" << __LINE__ << "]" << std::endl
//#define logToStream(message_, stream_) stream_ << "[" << getTimeString(time(nullptr)) << " T" << std::this_thread::get_id() << "] " << message_ << " [in " << __FILE__ << ":" << __LINE__ << "]" << std::endl

/**
 * @brief Logs a message to the log file.
 */
#define logToFile(message_) { std::lock_guard<std::mutex> lock(LOG_FILE_MUTEX); logToStream(message_, LOG_FILE); }

/**
 * @brief Logs a message to both the log file and the standard error output.
 */
#define logToFileAndConsole(message_) logToFile(message_); logToStream(message_, std::cerr)

#endif // ERROR_H
