#ifndef ALENKASIGNAL_MONTAGE_H
#define ALENKASIGNAL_MONTAGE_H

#include "openclprogram.h"

#include <CL/cl_gl.h>

#include <string>

namespace AlenkaSignal
{

// TODO: prohibit copying of this object

/**
 * @brief A class for creating kernel program for montage computation.
 *
 * This class generates a source string for a kernel function called 'montage'.
 * For this it uses the formulas passed to it in the constructor, the
 * montageHeader.cl file and string constants hard-coded in the cpp file.
 *
 * Then this string is used for creating an OpenCL kernel object that is
 * the final representation of the code. After the kernel object is retrieved
 * this object can be safely destroyed.
 */
template<class T>
class Montage
{
	OpenCLProgram* program = nullptr;
	cl_kernel kernel = nullptr;
	cl_int index;

public:
	/**
	 * @brief Montage constructor.
	 * @param sources OpenCL source code of the montage.
	 */
	Montage(const std::string& source, OpenCLContext* context, const std::string& headerSource = "");
	Montage(const std::vector<unsigned char>* binary, OpenCLContext* context);
	~Montage();

	/**
	 * @brief Returns the kernel object needed for execution of the code.
	 */
	cl_kernel getKernel()
	{
		if (kernel == nullptr)
			kernel = program->createKernel("montage");
		return kernel;
	}

	std::vector<unsigned char>* getBinary()
	{
		if (program)
			return program->getBinary();
		else
			return nullptr;
	}

	bool isCopyMontage() const { return program == nullptr; }
	cl_int copyMontageIndex() const { return index; }

	/**
	 * @brief Tests the source code of the montage.
	 * @param source String formula for one track.
	 * @param errorMessage [out] If not nullptr and an error is detected, an error message is stored here.
	 * @return True if the test succeeds.
	 */
	static bool test(const std::string& source, OpenCLContext* context, std::string* errorMessage = nullptr, const std::string& headerSource = "");
	
	/**
	 * @brief Removes single line and block comments from OpenCL code.
	 */
	static std::string stripComments(const std::string& code);
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_MONTAGE_H
