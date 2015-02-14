#ifndef ERROR_H
#define ERROR_H

#include <sstream>
#include <stdexcept>

namespace
{
template <typename T>
void printEC(T val, std::stringstream& ss)
{
    ss << std::dec << val << "(0x" << std::hex << val << std::dec << ")";
}

template <typename T>
void CEC(T val, T expected, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Unexpected error code: ";
	printEC(val, ss);
	ss << ", required ";
	printEC(expected, ss);
	ss << ". ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}

template <typename T>
void CNEC(T val, std::string message, const char* file, int line)
{
	std::stringstream ss;

	ss << "Error code returned ";
	printEC(val, ss);
	ss << ". ";

	ss << message << " " << file << ":" << line;

	throw std::runtime_error(ss.str());
}
}

#define checkErrorCode(val_, expected_, message_) if(val_ != expected_) { std::stringstream ss; ss << message_; CEC(val_, expected_, ss.str(), __FILE__, __LINE__); }
#define checkNotErrorCode(val_, expected_, message_) if(val_ == expected_) { std::stringstream ss; ss << message_; CNEC(val_, ss.str(), __FILE__, __LINE__); }

#endif // ERROR_H
