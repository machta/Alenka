
// Include Files
#include "spike_cluster_cpp.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp_data.h"
#include "spike_cluster_cpp_rtwutil.h"
#include "spike_cluster_cpp_terminate.h"

// Function Definitions

//
// Arguments    : void
// Return Type  : void
//
void spike_cluster_cpp_terminate() {
  emlrtFreeThreadStackData();
  omp_destroy_nest_lock(&emlrtNestLockGlobal);
}
