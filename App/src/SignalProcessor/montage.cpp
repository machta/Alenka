#include "montage.h"

#include <sstream>
#include <fstream>

using namespace std;

Montage::Montage(const vector<string>& sources, OpenCLContext* context) : numberOfRows(sources.size())
{
	program = new OpenCLProgram(buildSource(sources), context);
}

Montage::~Montage()
{
	delete program;
}

bool Montage::test(const string& source, OpenCLContext* context, string* errorMessage)
{
	// Use the OpenCL compiler to test the source.
	OpenCLProgram program(buildSource(vector<string> {source}), context);

	if (program.compilationSuccessful())
	{
		cl_kernel kernel = program.createKernel("montage");

		cl_int err = clReleaseKernel(kernel);
		checkErrorCode(err, CL_SUCCESS, "clReleaseKernel()");

		return true;
	}
	else
	{
		if (errorMessage != nullptr)
		{
			*errorMessage = "Compilation failed:\n" + program.getCompilationLog();
		}
		return false;
	}
}

string Montage::readHeader()
{
	string str;

	ifstream fs("montageHeader.cl");

	while (fs.peek() != EOF)
	{
		str.push_back(fs.get());
	}

	return str;
}

string Montage::buildSource(const vector<string>& sources)
{
	string src;

	src += readHeader();

	src += "\n__kernel void montage(__global float4* input, __global float4* output, int inputRowLength, int inputRowOffset, int outputRowLength)\n{\n";

	for (unsigned int i = 0; i < sources.size(); ++i)
	{
		src += montageRow(i, sources[i]);
	}

	src += "\n}\n";

	return src;
}

string Montage::montageRow(unsigned int row, const string& code)
{
	stringstream ss;

	ss << "{" << endl;
	ss << "float4 out = 0;" << endl;
	ss << "{" << endl;
	ss << code << endl;
	ss << "}" << endl;

	if (PROGRAM_OPTIONS["eventRenderMode"].as<int>() == 1)
	{
		ss << "output[outputRowLength*" << row << " + get_global_id(0)] = out;" << endl;
	}
	else
	{
		ss << "float4 output1 = out.s0011;" << endl;
		ss << "float4 output2 = out.s2233;" << endl;
		ss << "int outputIndex = 2*(outputRowLength*" << row << " + get_global_id(0));" << endl;
		ss << "output[outputIndex] = output1;" << endl;
		ss << "output[outputIndex + 1] = output2;" << endl;
	}

	ss << "}" << endl;

	return ss.str();
}
