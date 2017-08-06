
// Include Files
#include "spike_cluster_cpp.h"
#include "eml_rand.h"
#include "eml_rand_mt19937ar_stateful.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp_data.h"
#include "spike_cluster_cpp_initialize.h"
#include "spike_cluster_cpp_rtwutil.h"

// Function Definitions

//
// Arguments    : void
// Return Type  : void
//
void spike_cluster_cpp_initialize() {
  rt_InitInfAndNaN(8U);
  omp_init_nest_lock(&emlrtNestLockGlobal);
  getThreadID_init();
  eml_rand_init();
  c_eml_rand_mt19937ar_stateful_i();
  emlrtInitThreadStackData();
}
