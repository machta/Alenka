#include "../include/AlenkaSignal/montage.h"

#include "../include/AlenkaSignal/openclcontext.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <regex>
#include <sstream>

using namespace std;

namespace {

string indentLines(const string &text, int indentLevel) {
  string line, output, indent(2 * indentLevel, ' ');
  stringstream ss(text);

  while (getline(ss, line), ss)
    output += indent + line + "\n";

  return output;
}

template <class T>
string buildSource(const string &source, const string &headerSource = "",
                   const string &additionalParameters = "") {
  // The NAN value makes the signal line disappear, which makes it apparent that
  // the user made a mistake. But it caused problems during compilation on some
  // platforms, so I replaced it with 0.
  string src;

  if (is_same<T, double>::value)
    src += "#define float double\n\n";

  src += R"(
#define PARA                                                                   \
  __global float *_input_, int _inputRowLength_, int _inputRowOffset_,         \
     int _inputRowCount_
#define PASS _input_, _inputRowLength_, _inputRowOffset_, _inputRowCount_

float in(int i, PARA) {
  if (0 <= i && i < _inputRowCount_)
    return _input_[_inputRowLength_ * i + _inputRowOffset_ + get_global_id(0)];
  else
    return /*NAN*/ 0;
}
#define in(a_) in(a_, PASS)
)";
  src += headerSource;
  src += R"(

__kernel void montage(__global float* _input_, __global float* _output_,
                      int _inputRowLength_, int _inputRowOffset_,
                      int _inputRowCount_, int _outputRowLength_,
                      int _outputRowIndex_, int _outputCopyCount_)";
  src += additionalParameters + R"() {
  float out = 0;

  {
)";
  src += indentLines(source, 2);
  src += R"(  }

  int outputIndex = _outputCopyCount_ *
                    (_outputRowLength_ * _outputRowIndex_ + get_global_id(0));
  for (int i = 0; i < _outputCopyCount_; ++i) {
    _output_[outputIndex + i] = out;
  }
})";

  return src;
}

bool parseCopyMontage(const string &source, cl_int *index = nullptr) {
  try {
    const static regex re(R"(\s*out\s*=\s*in\s*\(\s*(\d+)\s*\)\s*;\s*)");

    smatch matches;
    bool res = regex_match(source, matches, re);

    if (res && index)
      *index = stoi(matches[1]);

    return res;
  } catch (regex_error) {
  }

  return false;
}

// Taken from here: https://stackoverflow.com/a/37516316/287933
template <class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT>
regex_replace_transform(BidirIt first, BidirIt last,
                        const std::basic_regex<CharT, Traits> &re,
                        UnaryFunction f) {
  std::basic_string<CharT> s;

  typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
  auto endOfLastMatch = first;

  auto callback = [&](const std::match_results<BidirIt> &match) {
    auto positionOfThisMatch = match.position(0);
    auto diff = positionOfThisMatch - positionOfLastMatch;

    auto startOfThisMatch = endOfLastMatch;
    std::advance(startOfThisMatch, diff);

    s.append(endOfLastMatch, startOfThisMatch);
    s.append(f(match));

    auto lengthOfMatch = match.length(0);

    positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

    endOfLastMatch = startOfThisMatch;
    std::advance(endOfLastMatch, lengthOfMatch);
  };

  std::sregex_iterator begin(first, last, re), end;
  std::for_each(begin, end, callback);

  s.append(endOfLastMatch, last);

  return s;
}

auto inLabelRegex() {
  const static regex re(R"(in\s*\(\s*(\"([^\\\"]|\\.)*\")\s*\))");
  // Tests of this regex: https://regex101.com/r/CNngSC/3
  return re;
}

string injectIndex(const smatch &m, const vector<string> &labels) {
  string mStr = m.str(0);
  smatch matches;
  bool res = regex_match(mStr, matches, inLabelRegex());
  assert(res);
  (void)res;

  string label = matches[1];
  label = label.substr(1, label.size() - 2);
  auto it = find(labels.begin(), labels.end(), label);

  int index;
  if (it == labels.end())
    index = -1;
  else
    index = distance(labels.begin(), it);

  return "in(" + to_string(index) + ")";
}

string replaceLabels(const string &source, const vector<string> &labels) {
  try {
    return regex_replace_transform(
        source.cbegin(), source.cend(), inLabelRegex(),
        [&labels](const smatch &m) { return injectIndex(m, labels); });
  } catch (regex_error) {
  }

  cerr << "Montage compilation error: this build dowsn't support std::regex"
       << endl;
  return "";
}

} // namespace

namespace AlenkaSignal {

template <class T>
Montage<T>::Montage(const string &source, OpenCLContext *context,
                    const string &headerSource,
                    const std::vector<string> &labels)
    : context(context) {
  string src = preprocessSource(source, labels);
  copyMontage = parseCopyMontage(src, &index);

  if (!copyMontage)
    this->source = stripComments(buildSource<T>(src, headerSource));
}

template <class T> Montage<T>::~Montage() {
  if (kernel) {
    cl_int err = clReleaseKernel(kernel);
    checkClErrorCode(err, "clReleaseKernel()");
  }
}

template <class T>
bool Montage<T>::test(const string &source, OpenCLContext *context,
                      string *errorMessage, const string &headerSource) {
  // logToFile("Testing montage code.");

  if (parseCopyMontage(source))
    return true;

  // Use the OpenCL compiler to test the source.
  OpenCLProgram program(buildSource<T>(source, headerSource), context);

  if (program.compilationSuccessful()) {
    cl_kernel kernel = program.createKernel("montage");

    cl_int err = clReleaseKernel(kernel);
    checkClErrorCode(err, "clReleaseKernel()");

    return true;
  }

  if (errorMessage != nullptr) {
    *errorMessage = "Compilation failed:\n" + program.getCompilationLog();
  }

  return false;
}

template <class T> string Montage<T>::stripComments(const string &code) {
  try {
    const static regex commentre(R"((/\*([^*]|(\*+[^*/]))*\*+/)|(//.*))");
    return regex_replace(code, commentre, string(""));
  } catch (regex_error) {
  }

  return code;
}

template <class T>
string Montage<T>::preprocessSource(const string &source,
                                    const vector<string> &labels) {
  return replaceLabels(stripComments(source), labels);
}

template <class T> void Montage<T>::buildProgram() {
  if (kernel || program)
    return; // The program is already built.

  if (copyMontage) {
    if (is_same<T, double>::value) {
      if (!context->hasCopyOnlyKernelDouble()) {
        string src =
            buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

        auto p = make_unique<OpenCLProgram>(src, context);
        if (!p->compilationSuccessful())
          cerr << "Copy only kernel compilation error: " << endl
               << p->getCompilationLog();

        context->setCopyOnlyKernelDouble(move(p));
      }

      kernel = context->copyOnlyKernelDouble();
    } else {
      if (!context->hasCopyOnlyKernelFloat()) {
        string src =
            buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

        auto p = make_unique<OpenCLProgram>(src, context);
        if (!p->compilationSuccessful())
          cerr << "Copy only kernel compilation error: " << endl
               << p->getCompilationLog();

        context->setCopyOnlyKernelFloat(move(p));
      }

      kernel = context->copyOnlyKernelFloat();
    }
  } else {
    program = make_unique<OpenCLProgram>(source, context);
  }
}

template class Montage<float>;
template class Montage<double>;

} // namespace AlenkaSignal
