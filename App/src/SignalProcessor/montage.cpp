#include "montage.h"

#include <sstream>
#include <fstream>

using namespace std;

Montage::Montage(const vector<string>& sources, OpenCLContext* context) : numberOfRows(sources.size())
{
	logToFile("Constructing montage.");

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
		checkClErrorCode(err, "clReleaseKernel()");

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
	// TODO: add proper indentation
	string src;

	src += "typedef float4 in_type;\n\n"
		   "#define PARA __global in_type* _input_, int _inputRowLength_, int _inputRowOffset_, int _channelsInFile_\n"
		   "#define PASS _input_, _inputRowLength_, _inputRowOffset_, _channelsInFile_\n\n"
		   "in_type in(int i, PARA) { return 0 <= i && i < _channelsInFile_ ? _input_[_inputRowLength_*i + _inputRowOffset_ + get_global_id(0)] : NAN; }\n"
		   "#define in(a_) in(a_, PASS)\n\n";

	src += readHeader();

	src += "\n\n__kernel void montage(__global in_type* _input_, __global in_type* _output_, int _inputRowLength_, int _inputRowOffset_, int _outputRowLength_, int _channelsInFile_)\n{\n\n";

	for (unsigned int i = 0; i < sources.size(); ++i)
	{
		src += montageRow(i, sources[i]);
	}

	src += "}\n";

	QString qsrc = QString::fromStdString(src);

	return src;
}

string Montage::montageRow(unsigned int row, const string& code)
{
	stringstream ss;

	ss << "{" << endl;
	ss << "in_type out = 0;" << endl;
	ss << "{" << endl;
	ss << code << endl;
	ss << "}" << endl;

	if (PROGRAM_OPTIONS["eventRenderMode"].as<int>() == 1)
	{
		ss << "_output_[_outputRowLength_*" << row << " + get_global_id(0)] = out;" << endl;
	}
	else
	{
		ss << "in_type output1 = out.s0011;" << endl;
		ss << "in_type output2 = out.s2233;" << endl;
		ss << "int outputIndex = 2*(_outputRowLength_*" << row << " + get_global_id(0));" << endl;
		ss << "_output_[outputIndex] = output1;" << endl;
		ss << "_output_[outputIndex + 1] = output2;" << endl;
	}

	ss << "}" << endl << endl;

	return ss.str();
}
