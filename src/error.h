/**
 * @brief This file defines global auxiliary functions and macros
 * mostly for the purpose of error handling and reporting.
 *
 * @file
 */

#ifndef ERROR_H
#define ERROR_H

#include "options.h"

#include <AlenkaSignal/openclcontext.h>

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

std::string getTimeString(std::time_t t)
{
	(void)getTimeString;

	std::string tmp = asctime(localtime(&t));

	if (tmp.back() == '\n')
		tmp.pop_back();

	return tmp;
}

} // namespace

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

template<class... T>
inline void printBuffer(T... p)
{
	if (PROGRAM_OPTIONS.isSet("printBuffers"))
		AlenkaSignal::OpenCLContext::printBuffer(p...);
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
 *
 * The message_ parameter of can be an expressions like:
 * * "some string message"
 * * "printing the value of a variable interVar: " << integerVar
 * * "a string" << std::endl << "some more on a new line"
 */
#define logToFile(message_) { std::lock_guard<std::mutex> lock(LOG_FILE_MUTEX); logToStream(message_, LOG_FILE); }

/**
 * @brief Logs a message to both the log file and the standard error output.
 *
 * message_ works the same as for logToFile().
 */
#define logToFileAndConsole(message_) { logToFile(message_); logToStream(message_, std::cerr); }

#endif // ERROR_H
