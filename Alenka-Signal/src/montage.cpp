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

#include <detailedexception.h>

using namespace std;

namespace {

template <class T> string typeStr() {
  string res;

  if (is_same<T, float>::value)
    res = "(float)";
  else if (is_same<T, double>::value)
    res = "(double)";
  else
    assert(false);

  return res;
}

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
     int IN_COUNT, __global float *_xyz_, int drawIndex, int INDEX
#define PASS _input_, _inputRowLength_, _inputRowOffset_, IN_COUNT, _xyz_,     \
  drawIndex, INDEX

// Input: value of a sample for channel i.
float in(int i, PARA) {
  if (0 <= i && i < IN_COUNT)
    return _input_[_inputRowLength_ * i + _inputRowOffset_ + get_global_id(0)];
  else
    return /*NAN*/ 0;
}
#define in(a_) in(a_, PASS)

float x(int i, PARA) {
  if (0 <= i && i < IN_COUNT)
    return _xyz_[3 * i];
  else
    return /*NAN*/ 0;
}
#define x(a_) x(a_, PASS)

float y(int i, PARA) {
  if (0 <= i && i < IN_COUNT)
    return _xyz_[3 * i + 1];
  else
    return /*NAN*/ 0;
}
#define y(a_) y(a_, PASS)

float z(int i, PARA) {
  if (0 <= i && i < IN_COUNT)
    return _xyz_[3 * i + 2];
  else
    return /*NAN*/ 0;
}
#define z(a_) z(a_, PASS)
)";
  src += headerSource;
  src += R"(

__kernel void montage(__global float *_input_, __global float *_output_,
                      int _inputRowLength_, int _inputRowOffset_,
                      int IN_COUNT, int _outputRowLength_, int drawIndex,
                      int INDEX, int _outputCopyCount_,
                      __global float *_xyz_)";
  src += additionalParameters + R"() {
  float out = 0;

  {
)";
  src += indentLines(source, 2);
  src += R"(  }

  int outputIndex = _outputCopyCount_ *
                    (_outputRowLength_ * drawIndex + get_global_id(0));
  for (int i = 0; i < _outputCopyCount_; ++i) {
    _output_[outputIndex + i] = out;
  }
})";

  return src;
}

/**
 * @brief Tries to match an identity-montage.
 *
 * An identity-montage looks exactly like this:
 * ```
 * out = in(INDEX);
 * ```
 *
 * Tests of the regex are [here](https://regex101.com/r/P2RlrZ/1).
 */
bool parseIdentityMontage(const string &source) {
  try {
    const static regex re(R"(\s*out\s*=\s*in\s*\(\s*INDEX\s*\)\s*;\s*)");

    smatch matches;
    return regex_match(source, matches, re);
  } catch (regex_error) {
    // Intentionally left empty to silence the errors due to missing support.
  }

  return false;
}

/**
 * @brief Tries to match a trivial copy-montage.
 *
 * An example of a copy montage:
 * ```
 * out = in(5);
 * ```
 *
 * Tests of the regex are [here](https://regex101.com/r/TWqWyU/1).
 */
bool parseCopyMontage(const string &source, cl_int *index = nullptr) {
  try {
    const static regex re(R"(\s*out\s*=\s*in\s*\(\s*(\d+)\s*\)\s*;\s*)");

    smatch matches;
    bool res = regex_match(source, matches, re);

    if (res && index)
      *index = stoi(matches[1]);

    return res;
  } catch (regex_error) {
    // Intentionally left empty to silence the errors due to missing support.
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

struct LabelRegex {
  int paramCount;
  regex re;
};

/**
 * @brief Return regular expressions for label replacement.
 *
 * This expression matches a C-like identifier followed by parentheses enclosing
 * paramCount string-literal parameters.
 *
 * Tests of the regex:
 * * [for paramCount=1](https://regex101.com/r/41wEzp/1)
 * * [for paramCount=2](https://regex101.com/r/kQCLwW/1)
 */
LabelRegex makeLabelRegex(int paramCount) {
  const string front = R"(([_a-zA-Z][_a-zA-Z0-9]{0,30})\s*\()";
  const string stringLiteral = R"(\s*\"(([^\\\"]|\\.)*)\"\s*)";
  const string back = R"(\))";
  assert(0 < paramCount);

  string expression = front + stringLiteral;
  for (int i = 1; i < paramCount; ++i)
    expression += "," + stringLiteral;
  expression += back;

  return {paramCount, regex(expression)};
}

string injectIndex(const smatch &m, const vector<string> &labels,
                   const LabelRegex &labelRegex) {
  string mStr = m.str(0);
  smatch matches;
  // matches[1] = the first group -- the identifier
  // matches[2, 4, ...] = the string parameters

  bool res = regex_match(mStr, matches, labelRegex.re);
  assert(res);
  (void)res;

  vector<int> indexes;
  indexes.reserve(labelRegex.paramCount);

  for (int i = 1; i <= labelRegex.paramCount; ++i) {
    auto it = find(labels.begin(), labels.end(), matches[2 * i]);
    indexes.push_back(it == labels.end()
                          ? -1
                          : static_cast<int>(distance(labels.begin(), it)));
  }

  string result = matches[1];
  result += '(';

  assert(0 < labelRegex.paramCount);
  result += to_string(indexes[0]);

  for (unsigned int i = 1; i < indexes.size(); ++i)
    result += ", " + to_string(indexes[i]);

  return result + ")";
}

string replaceLabels(const string &source, const vector<string> &labels,
                     const LabelRegex &labelRegex) {
  try {
    return regex_replace_transform(source.cbegin(), source.cend(),
                                   labelRegex.re,
                                   [&labels, &labelRegex](const smatch &m) {
                                     return injectIndex(m, labels, labelRegex);
                                   });
  } catch (regex_error) {
    // Intentionally left empty to silence the errors due to missing support.
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

  if (parseIdentityMontage(src))
    montageType = IdentityMontage;
  else if (parseCopyMontage(src, &copyIndex))
    montageType = CopyMontage;
  else
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
                      string *errorMessage) {
  if (parseIdentityMontage(source) || parseCopyMontage(source))
    return true;

  return testHeader(source, context, "", errorMessage);
}

/**
 * @brief Remove C and C++ comments from code.
 *
 * Tests of the regex are [here](https://regex101.com/r/HoZ7v5/1).
 */
template <class T> string Montage<T>::stripComments(const string &code) {
  try {
    const static regex re(R"((/\*([^*]|(\*+[^*/]))*\*+/)|(//.*))");
    return regex_replace(code, re, string(""));
  } catch (regex_error) {
    // Intentionally left empty to silence the errors due to missing support.
  }

  return code; // TODO: Remove consecutive empty lines that sometimes appear
               // when you remove multiline comments.
}

template <class T>
bool Montage<T>::testHeader(const string &source, OpenCLContext *context,
                            const string &headerSource, string *errorMessage) {

  // Use the OpenCL compiler to test the source.
  OpenCLProgram program(buildSource<T>(source, headerSource), context);

  if (CL_SUCCESS == program.compileStatus()) {
    cl_kernel kernel = program.createKernel("montage");

    cl_int err = clReleaseKernel(kernel);
    checkClErrorCode(err, "clReleaseKernel()");

    return true;
  }

  if (errorMessage)
    *errorMessage = "Compilation failed:\n" + program.getCompileLog();

  return false;
}

template <class T>
string Montage<T>::preprocessSource(const string &source,
                                    const vector<string> &labels) {
  string src = stripComments(source);

  const static LabelRegex labelRegex1 = makeLabelRegex(1);
  const static LabelRegex labelRegex2 = makeLabelRegex(2);
  src = replaceLabels(src, labels, labelRegex1);
  src = replaceLabels(src, labels, labelRegex2);

  return src;
}

template <class T> void Montage<T>::buildProgram() {
  if (kernel || program)
    return; // The program is already built.

  switch (montageType) {
  case IdentityMontage:
    buildIdentityProgram();
    break;
  case CopyMontage:
    buildCopyProgram();
    break;
  case NormalMontage:
    program = make_unique<OpenCLProgram>(source, context);

    if (CL_SUCCESS != program->compileStatus()) {
      const string msg = "Kernel " + typeStr<T>();
      throwDetailed(runtime_error(program->makeErrorMessage(msg)));
    }
    break;
  default:
    assert(false && "Unexpected montage type");
    break;
  }
}

template <class T> void Montage<T>::buildCopyProgram() {
  const bool isDouble = is_same<T, double>::value;

  if (isDouble ? !context->hasCopyOnlyKernelDouble()
               : !context->hasCopyOnlyKernelFloat()) {
    auto p = make_unique<OpenCLProgram>(
        buildSource<T>("out = in(_copyIndex_);", "", ", int _copyIndex_"),
        context);

    if (CL_SUCCESS != p->compileStatus()) {
      const string msg = "Copy kernel " + typeStr<T>();
      throwDetailed(runtime_error(program->makeErrorMessage(msg)));
    }

    isDouble ? context->setCopyOnlyKernelDouble(move(p))
             : context->setCopyOnlyKernelFloat(move(p));
  }

  kernel = isDouble ? context->copyOnlyKernelDouble()
                    : context->copyOnlyKernelFloat();
}

template <class T> void Montage<T>::buildIdentityProgram() {
  const bool isDouble = is_same<T, double>::value;

  if (isDouble ? !context->hasIdentityKernelDouble()
               : !context->hasIdentityKernelFloat()) {
    auto p =
        make_unique<OpenCLProgram>(buildSource<T>("out = in(INDEX);"), context);

    if (CL_SUCCESS != p->compileStatus()) {
      const string msg = "Identity kernel " + typeStr<T>();
      throwDetailed(runtime_error(program->makeErrorMessage(msg)));
    }

    isDouble ? context->setIdentityKernelDouble(move(p))
             : context->setIdentityKernelFloat(move(p));
  }

  kernel = isDouble ? context->identityKernelDouble()
                    : context->identityKernelFloat();
}

template class Montage<float>;
template class Montage<double>;

} // namespace AlenkaSignal
