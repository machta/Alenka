
// Include Files
#include "spike_cluster_cpp.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp_rtwutil.h"

// Variable Definitions
static spike_cluster_cppTLS *spike_cluster_cppTLSGlobal;

#pragma omp threadprivate(spike_cluster_cppTLSGlobal)

// Function Definitions

//
// Arguments    : void
// Return Type  : void
//
void emlrtFreeThreadStackData() {
  int i;

#pragma omp parallel for schedule(static) num_threads(omp_get_max_threads())

  for (i = 1; i <= omp_get_max_threads(); i++) {
    free(spike_cluster_cppTLSGlobal);
  }
}

//
// Arguments    : void
// Return Type  : spike_cluster_cppTLS *
//
spike_cluster_cppTLS *emlrtGetThreadStackData() {
  return spike_cluster_cppTLSGlobal;
}

//
// Arguments    : void
// Return Type  : void
//
void emlrtInitThreadStackData() {
  int i;

#pragma omp parallel for schedule(static) num_threads(omp_get_max_threads())

  for (i = 1; i <= omp_get_max_threads(); i++) {
    spike_cluster_cppTLSGlobal =
        (spike_cluster_cppTLS *)malloc(1U * sizeof(spike_cluster_cppTLS));
  }
}

//
// Arguments    : double u
// Return Type  : double
//
double rt_roundd_snf(double u) {
  double y;
  if (std::abs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = std::floor(u + 0.5);
    } else if (u > -0.5) {
      y = u * 0.0;
    } else {
      y = std::ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}
