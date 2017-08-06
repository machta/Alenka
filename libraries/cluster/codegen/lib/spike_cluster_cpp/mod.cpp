
// Include Files
#include "mod.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_rtwutil.h"

// Function Definitions

//
// Arguments    : double x
//                double y
// Return Type  : double
//
double b_mod(double x, double y) {
  double r;
  if (y == std::floor(y)) {
    r = x - std::floor(x / y) * y;
  } else {
    r = x / y;
    if (std::abs(r - rt_roundd_snf(r)) <=
        2.2204460492503131E-16 * std::abs(r)) {
      r = 0.0;
    } else {
      r = (r - std::floor(r)) * y;
    }
  }

  return r;
}
