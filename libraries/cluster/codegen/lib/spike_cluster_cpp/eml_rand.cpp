
// Include Files
#include "eml_rand.h"
#include "eml_rand_mt19937ar_stateful.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Variable Definitions
static unsigned int method;

#pragma omp threadprivate(method)

static boolean_T method_not_empty = false;

#pragma omp threadprivate(method_not_empty)

static unsigned int method_master;
static boolean_T method_not_empty_master = false;

// Function Definitions

//
// Arguments    : double varargin_2
//                emxArray_real_T *r
// Return Type  : void
//
void b_eml_rand(double varargin_2, emxArray_real_T *r) {
  b_eml_rand_mt19937ar_stateful(varargin_2, r);
}

//
// Arguments    : void
// Return Type  : double
//
double eml_rand() { return eml_rand_mt19937ar_stateful(); }

//
// Arguments    : void
// Return Type  : void
//
void eml_rand_init() {
  int ub_loop;
  int i;
  ub_loop = omp_get_max_threads();

#pragma omp parallel for schedule(static) num_threads(omp_get_max_threads())

  for (i = 1; i <= ub_loop; i++) {
    method = 7U;
    method_not_empty = true;
  }

  method_master = method;
  method = 7U;
  method_not_empty = true;
}

//
// Arguments    : void
// Return Type  : void
//
void eml_rand_swap() {
  unsigned int method_tmp;
  boolean_T method_not_empty_tmp;
  method_tmp = method;
  method = method_master;
  method_master = method_tmp;
  method_not_empty_tmp = method_not_empty;
  method_not_empty = method_not_empty_master;
  method_not_empty_master = method_not_empty_tmp;
}
