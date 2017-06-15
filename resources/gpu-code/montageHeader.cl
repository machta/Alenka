// TODO: Move these comments somewhere, then delete this file.

/**
 * @brief In this file the user can define custom functions for use in the
 * montage track code.
 *
 * The following example shows a custom function that sums the samples from
 * channels designated by the range [from, to].
 * @code{.cpp}

float sum(int from, int to, PARA) {
  float tmp = 0;

  for (int i = from; i <= to; ++i) {
    tmp += in(i);
  }

  return tmp;
}
#define sum(a_, b_) sum(a_, b_, PASS)

 * @endcode
 *
 * If the function accesses the input buffer (i.e. uses the in() macro)
 * some additional parameters must be specified.
 * All the user has to do to ensure that these parameters are correctly passed
 * is specify an additional parameter called PARA.
 * When this function is called PASS must be used in place of the PARA
 * parameter.
 *
 * In order to simplify the code formulas a macro with the same name can be
 * defined to hide the function definition. Then the function can be used as
 * intended (with just two parameters):
 * @code{.cpp}
out = sum(1, 3);
 * @endcode
 *
 * @file
 */
 /// @cond

float sum(int from, int to, PARA) {
  float tmp = 0;

  for (int i = from; i <= to; ++i) {
    tmp += in(i);
  }

  return tmp;
}
#define sum(a_, b_) sum(a_, b_, PASS)

/// @endcond
