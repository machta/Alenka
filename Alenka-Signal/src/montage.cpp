/**
  * @file
  */

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
     int _inputRowCount_, __global float *_xyz_
#define PASS _input_, _inputRowLength_, _inputRowOffset_, _inputRowCount_, _xyz_

// Input: value of a sample for channel i.
float in(int i, PARA) {
  if (0 <= i && i < _inputRowCount_)
    return _input_[_inputRowLength_ * i + _inputRowOffset_ + get_global_id(0)];
  else
    return /*NAN*/ 0;
}
#define in(a_) in(a_, PASS)

float x(int i, PARA) {
  if (0 <= i && i < _inputRowCount_)
    return _xyz_[3 * i];
  else
    return /*NAN*/ 0;
}
#define x(a_) x(a_, PASS)

float y(int i, PARA) {
  if (0 <= i && i < _inputRowCount_)
    return _xyz_[3 * i + 1];
  else
    return /*NAN*/ 0;
}
#define y(a_) y(a_, PASS)

float z(int i, PARA) {
  if (0 <= i && i < _inputRowCount_)
    return _xyz_[3 * i + 2];
  else
    return /*NAN*/ 0;
}
#define z(a_) z(a_, PASS)

// Euclidean distance between channels i and j.
float dist(int i, int j, PARA) {
  return sqrt(pown(x(i) - x(j), 2) + pown(y(i) - y(j), 2) +
              pown(z(i) - z(j), 2));
}
#define dist(a_, b_) dist(a_, b_, PASS)
)";
  src += headerSource;
  src += R"(

__kernel void montage(__global float *_input_, __global float *_output_,
                      int _inputRowLength_, int _inputRowOffset_,
                      int _inputRowCount_, int _outputRowLength_,
                      int _outputRowIndex_, int _outputCopyCount_,
                      __global float *_xyz_)";
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
basic_string<CharT>
regex_replace_transform(BidirIt first, BidirIt last,
                        const basic_regex<CharT, Traits> &re, UnaryFunction f) {
  basic_string<CharT> s;

  typename match_results<BidirIt>::difference_type positionOfLastMatch = 0;
  auto endOfLastMatch = first;

  auto callback = [&](const match_results<BidirIt> &match) {
    auto positionOfThisMatch = match.position(0);
    auto diff = positionOfThisMatch - positionOfLastMatch;

    auto startOfThisMatch = endOfLastMatch;
    advance(startOfThisMatch, diff);

    s.append(endOfLastMatch, startOfThisMatch);
    s.append(f(match));

    auto lengthOfMatch = match.length(0);

    positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

    endOfLastMatch = startOfThisMatch;
    advance(endOfLastMatch, lengthOfMatch);
  };

  sregex_iterator begin(first, last, re), end;
  for_each(begin, end, callback);

  s.append(endOfLastMatch, last);

  return s;
}

/**
 * @brief Return regular expressions for label replacement.
 *
 * This expression matches a C-like identifier followed by parentheses enclosing
 * paramCount string-literal parameters.
 *
 * Tests of the expressions:
 * * [for paramCount=1](https://regex101.com/r/41wEzp/1)
 * * [for paramCount=2](https://regex101.com/r/kQCLwW/1)
 */
regex labelRegex(int paramCount) {
  const string front = R"(([_a-zA-Z][_a-zA-Z0-9]{0,30})\s*\()";
  const string stringLiteral = R"(\s*\"(([^\\\"]|\\.)*)\"\s*)";
  const string back = R"(\))";
  assert(0 < paramCount);

  string expression = front + stringLiteral;
  for (int i = 1; i < paramCount; ++i)
    expression += "," + stringLiteral;
  expression += back;

  return regex(expression);
}

string injectIndex(const smatch &m, const vector<string> &labels,
                   int paramCount) {
  string mStr = m.str(0);
  smatch matches;
  // matches[1] = the first group -- the identifier
  // matches[2, 4, ...] = the string parameters

  bool res = regex_match(mStr, matches, labelRegex(paramCount));
  assert(res);
  (void)res;

  vector<int> indexes;
  indexes.reserve(paramCount);

  for (int i = 1; i <= paramCount; ++i) {
    auto it = find(labels.begin(), labels.end(), matches[2 * i]);
    indexes.push_back(it == labels.end()
                          ? -1
                          : static_cast<int>(distance(labels.begin(), it)));
  }

  string result = matches[1];
  result += '(';

  assert(0 < paramCount);
  result += to_string(indexes[0]);

  for (unsigned int i = 1; i < indexes.size(); ++i)
    result += ", " + to_string(indexes[i]);

  return result + ")";
}

string replaceLabels(const string &source, const vector<string> &labels,
                     int paramCount) {
  try {
    return regex_replace_transform(source.cbegin(), source.cend(),
                                   labelRegex(paramCount),
                                   [&labels, paramCount](const smatch &m) {
                                     return injectIndex(m, labels, paramCount);
                                   });
  } catch (regex_error) {
  }

  cerr << "Montage compilation error: this build doesn't support std::regex"
       << endl;
  return "";
}

} // namespace

namespace AlenkaSignal {

template <class T>
Montage<T>::Montage(const string &source, OpenCLContext *context,
                    const string &headerSource, const vector<string> &labels)
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

  if (program.compileSuccess()) {
    cl_kernel kernel = program.createKernel("montage");

    cl_int err = clReleaseKernel(kernel);
    checkClErrorCode(err, "clReleaseKernel()");

    return true;
  }

  if (errorMessage) {
    *errorMessage = "Compilation failed:\n" + program.getCompileLog();
  }

  return false;
}

template <class T> string Montage<T>::stripComments(const string &code) {
  try {
    const static regex commentre(R"((/\*([^*]|(\*+[^*/]))*\*+/)|(//.*))");
    return regex_replace(code, commentre, string(""));
  } catch (regex_error) {
  }

  return code; // TODO: Remove consecutive empty lines that sometimes appear
               // when you remove multiline comments.
}

template <class T>
string Montage<T>::preprocessSource(const string &source,
                                    const vector<string> &labels) {
  string src = stripComments(source);
  src = replaceLabels(src, labels, 1);
  src = replaceLabels(src, labels, 2);
  return src;
}

template <class T> void Montage<T>::buildProgram() {
  if (kernel || program)
    return; // The program is already built.

  if (copyMontage) {
    buildCopyProgram();
  } else {
    program = make_unique<OpenCLProgram>(source, context);
    if (!program->compileSuccess()) {
      cerr << "Montage compilation error:\n" << program->getCompileLog();
      throw runtime_error("Montage: compilation failed");
    }
  }
}

template <class T> void Montage<T>::buildCopyProgram() {
  if (is_same<T, double>::value) {
    if (!context->hasCopyOnlyKernelDouble()) {
      string src =
          buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

      auto p = make_unique<OpenCLProgram>(src, context);
      if (!p->compileSuccess()) {
        cerr << "Copy only kernel compilation error:\n" << p->getCompileLog();
        throw runtime_error("Montage: compilation failed");
      }

      context->setCopyOnlyKernelDouble(move(p));
    }

    kernel = context->copyOnlyKernelDouble();
  } else {
    if (!context->hasCopyOnlyKernelFloat()) {
      string src =
          buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_");

      auto p = make_unique<OpenCLProgram>(src, context);
      if (!p->compileSuccess()) {
        cerr << "Copy only kernel compilation error:\n" << p->getCompileLog();
        throw runtime_error("Montage: compilation failed");
      }

      context->setCopyOnlyKernelFloat(move(p));
    }

    kernel = context->copyOnlyKernelFloat();
  }
}

template class Montage<float>;
template class Montage<double>;

} // namespace AlenkaSignal
