#include "../include/AlenkaSignal/montage.h"

#include "../include/AlenkaSignal/openclcontext.h"

#include <regex>
#include <iostream>
#include <sstream>

using namespace std;

namespace
{

template<class T>
string buildSource(const string& source, const string& headerSource = "", const string& additionalParameters = "")
{
	// The NAN value makes the signal line disappear, which makes it apparent that the user made a mistake.
	// But itt caused problems during compilation on some platforms, so I replaced it with 0.
	string src;

	if (is_same<T, double>::value)
		src += "#define float double\n\n";

	src += R"(#define PARA __global float* _input_, int _inputRowLength_, int _inputRowOffset_, int _inputRowCount_
#define PASS _input_, _inputRowLength_, _inputRowOffset_, _inputRowCount_

float in(int i, PARA)
{
	return 0 <= i && i < _inputRowCount_ ? _input_[_inputRowLength_*i + _inputRowOffset_ + get_global_id(0)] : /*NAN*/0;
}
#define in(a_) in(a_, PASS)
)";

	src += headerSource;

	src += R"(

__kernel void montage(__global float* _input_, __global float* _output_, int _inputRowLength_, int _inputRowOffset_, int _inputRowCount_, int _outputRowLength_, int _outputRowIndex_, int _outputCopyCount_)" + additionalParameters + R"()
{
	float out = 0;

	{
)";

	stringstream ss(source);
	string line;
	while (getline(ss, line), ss)
		src += "\t\t" + line + "\n";

	src += R"(	}

	int outputIndex = _outputCopyCount_*(_outputRowLength_*_outputRowIndex_ + get_global_id(0));
	for (int i = 0; i < _outputCopyCount_; ++i)
	{
		_output_[outputIndex + i] = out;
	}
})";

	return src;
}

bool parseCopyMontage(const string& source, cl_int* index = nullptr)
{
	try
	{
		const static regex re(R"(\s*out\s*=\s*in\s*\(\s*(\d+)\s*\)\s*;\s*)");

		smatch matches;
		bool res =  regex_match(source, matches, re);

		if (res && index)
			*index = stoi(matches[1]);

		return res;
	}
	catch (regex_error) {}
	return false;
}

} // namespace

namespace AlenkaSignal
{

template<class T>
Montage<T>::Montage(const string& source, OpenCLContext* context, const string& headerSource)
{
	if (parseCopyMontage(source, &index))
	{
		if (is_same<T, double>::value)
		{
			if (!context->hasCopyOnlyKernelDouble())
			{
				string src = buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

				OpenCLProgram* p = new OpenCLProgram(src, context);
				if (!p->compilationSuccessful())
					cerr << "Copy only kernel compilation error: " << endl << p->getCompilationLog();

				context->setCopyOnlyKernelDouble(p);
			}

			kernel = context->copyOnlyKernelDouble();
		}
		else
		{
			if (!context->hasCopyOnlyKernelFloat())
			{
				string src = buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

				OpenCLProgram* p = new OpenCLProgram(src, context);
				if (!p->compilationSuccessful())
					cerr << "Copy only kernel compilation error: " << endl << p->getCompilationLog();

				context->setCopyOnlyKernelFloat(p);
			}

			kernel = context->copyOnlyKernelFloat();
		}
	}
	else
	{
		program = new OpenCLProgram(buildSource<T>(source, headerSource), context);
	}
}

template<class T>
Montage<T>::Montage(const std::vector<unsigned char>* binary, OpenCLContext* context)
{
	program = new OpenCLProgram(binary, context);
}

template<class T>
Montage<T>::~Montage()
{
	if (kernel)
	{
		cl_int err = clReleaseKernel(kernel);
		checkClErrorCode(err, "clReleaseKernel()");
	}

	delete program;
}

template<class T>
bool Montage<T>::test(const string& source, OpenCLContext* context, string* errorMessage, const string& headerSource)
{
	//logToFile("Testing montage code.");

	if (parseCopyMontage(source))
		return true;

	// Use the OpenCL compiler to test the source.
	OpenCLProgram program(buildSource<T>(source, headerSource), context);

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

template<class T>
string Montage<T>::stripComments(const string& code)
{
	try
	{
		const static regex commentre(R"((/\*([^*]|(\*+[^*/]))*\*+/)|(//.*))");
		return regex_replace(code, commentre, string(""));
	}
	catch (regex_error) {}
	return code;
}

template class Montage<float>;
template class Montage<double>;

} // namespace AlenkaSignal
