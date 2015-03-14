#include "montage.h"

#include <sstream>
#include <fstream>

using namespace std;

Montage::Montage(const vector<string>& sources, OpenCLContext* context)
	: sources(sources), context(context)
{

}

Montage::~Montage()
{
}

string Montage::test(const string& source, OpenCLContext* context)
{
	// Use the OpenCL compiler to test the source.
	OpenCLProgram prog(buildSource(vector<string> {source}), context);

	if (prog.compilationSuccessful())
	{
		prog.createKernel("montage0");

		return ""; // Empty string means test was successful.
	}
	else
	{
		return "Compilation failed:\n" + prog.getCompilationLog();
	}
}

string Montage::buildSource(const vector<string>& sources)
{
	string src;

	ifstream fs("montageHeader.txt");
	while (fs.peek() != EOF)
	{
		src.push_back(fs.get());
	}

	for (unsigned int i = 0; i < sources.size(); ++i)
	{
		src += montage(i, sources[i]);
		src += "\n";
	}

	return src;
}

string Montage::montage(unsigned int row, const string& code)
{
	stringstream ss;

	ss << "__kernel void montage" << row << "(__global float4* input, __global float4* output, int inputRowLength, int inputRowOffset, int outputOffset)" << endl;
	ss << "{" << endl;
	ss << "float4 out = 0;" << endl;
	ss << code << endl;
	ss << "output[outputOffset + get_global_id(0)] = out;" << endl;
	ss << "}" << endl;

	return ss.str();
}

