
// Include Files
#include "rand.h"
#include "eml_rand.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : void
// Return Type  : double
//
double b_rand() { return eml_rand(); }

//
// Arguments    : double varargin_2
//                emxArray_real_T *r
// Return Type  : void
//
void c_rand(double varargin_2, emxArray_real_T *r) {
  b_eml_rand(varargin_2, r);
}
