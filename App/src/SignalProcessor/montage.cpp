#include "montage.h"

#include <sstream>

using namespace std;

namespace
{
string header()
{
	return "#define in(a_) input[inputRowLength*(a_) + inputRowOffset + get_global_id(0)]"
		   ""
		   "float4 sum(int from, int to, int inputRowLength, int inputRowOffset)"
		   "{"
		   "	float4 tmp = 0;"
		   "	for (int i = from; i <= to; ++i)"
		   "	{"
		   "		tmp += in(i);"
		   "	}"
		   "	return tmp;"
		   "}"
		   ""
		   "#define sum(a_, b_) sum(a_, b_, inputRowLength, inputRowOffset);";
}

string kernelDefinition()
{
	return "__kernel void montage(__global float4 input, __global float4 output, int inputRowLength, int inputRowOffset, int outputRowLength)";
}

string rowDefinitionStart()
{
	return "{"
		   "float4 out = 0;";
}

string rowDefinitionEnd(int row)
{
	stringstream ss;
	ss << "output[outputRowLength*" << row << " + get_global_id(0)] = out";
	ss << "}";

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
	src += "{";

	for (int i = 0; i < sources.size(); ++i)
	{
		src += rowDefinitionStart();
		src += sources[i];
		src += rowDefinitionEnd(i);
	}

	src += "}";

	return src;
}

