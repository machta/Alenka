#include "montage.h"

#include <sstream>

using namespace std;

namespace
{
string header()
{
	return "#define in(a_) input[inputRowLength*(a_) + inputRowOffset + get_global_id(0)]\n"
		   "\n"
		   "float4 sum(int from, int to, __global float4* input, int inputRowLength, int inputRowOffset)\n"
		   "{\n"
		   "	float4 tmp = 0;\n"
		   "	for (int i = from; i <= to; ++i)\n"
		   "	{\n"
		   "		tmp += in(i);\n"
		   "	}\n"
		   "	return tmp;\n"
		   "}\n"
		   "\n"
		   "#define sum(a_, b_) sum(a_, b_, input, inputRowLength, inputRowOffset)\n";
}

string kernelDefinition()
{
	return "__kernel void montage(__global float4* input, __global float4* output, int inputRowLength, int inputRowOffset, int outputRowLength)\n";
}

string rowDefinitionStart()
{
	return "{\n"
		   "float4 out = 0;\n";
}

string rowDefinitionEnd(int row)
{
	stringstream ss;
	ss << "output[outputRowLength*" << row << " + get_global_id(0)] = out;\n";
	ss << "}\n";

	return ss.str();
}
}

Montage::Montage(const vector<string>& sources, OpenCLContext* context)
{
	numberOfRows = sources.size();
	program = new OpenCLProgram(buildSource(sources).c_str(), context);
	kernel = program->createKernel("montage");
}

Montage::~Montage()
{
	delete program;

	cl_int err = clReleaseKernel(kernel);
	checkErrorCode(err, CL_SUCCESS, "clReleaseKernel()");
}

string Montage::test(const string& source, OpenCLContext* context)
{
	// Use the OpenCL compiler to test the source.
	OpenCLProgram prog(buildSource(vector<string> {source}).c_str(), context);

	if (prog.compilationSuccessful())
	{
		prog.createKernel("montage");

		return ""; // Empty string means test was successful.
	}
	else
	{
		return "Compilation failed:\n" + prog.getCompilationLog();
	}
}

string Montage::buildSource(const vector<string>& sources)
{
	string src = header();
	src += kernelDefinition();
	src += "{\n";

	for (unsigned int i = 0; i < sources.size(); ++i)
	{
		src += rowDefinitionStart();
		src += sources[i];
		src += "\n";
		src += rowDefinitionEnd(i);
	}

	src += "}\n";

	return src;
}

