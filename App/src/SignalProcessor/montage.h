#ifndef MONTAGE_H
#define MONTAGE_H

#include "../openclcontext.h"
#include "../openclprogram.h"
#include "../error.h"

#include <CL/cl_gl.h>

#include <vector>
#include <string>

/**
 * @brief A class for creating kernel program for montage computation.
 *
 * This class generates a source string for a kernel function called 'montage'.
 * For this it uses the formulas passed to it in the constructor, the
 * montageHeader.cl file and string constants hardcoded in the cpp file.
 *
 * Then this string is used for creating an OpenCL kernel object that is
 * the final representation of the code. After the kernel object is retrieved
 * this object can be safely destroyed.
 */
class Montage
{
public:
	/**
	 * @brief Montage constructor.
	 * @param sources Vector of string formulas for individual tracks.
	 */
	Montage(const std::vector<std::string>& sources, OpenCLContext* context);
	~Montage();

	/**
	 * @brief Returns the number of tracks of the montage. 
	 */
	unsigned int getNumberOfRows() const
	{
		return numberOfRows;
	}
	
	/**
	 * @brief Returns the kernel object needed for execution of the code.
	 */
	cl_kernel getKernel()
	{
		return program->createKernel("montage");
	}

	/**
	 * @brief Tests the source code of the montage.
	 * @param source String formula for one track.
	 * @param errorMessage [out] If not nullptr and an error is detected, an error message is stored here.
	 * @return True if the test succeeds.
	 */
	static bool test(const std::string& source, OpenCLContext* context, std::string* errorMessage = nullptr);
	
	/**
	 * @brief Returns the text of the montageHeader.cl file.
	 */
	static std::string readHeader();

private:
	unsigned int numberOfRows;
	OpenCLProgram* program;

	static std::string buildSource(const std::vector<std::string>& sources);
	static std::string montageRow(unsigned int row, const std::string& code);
};

#endif // MONTAGE_H
