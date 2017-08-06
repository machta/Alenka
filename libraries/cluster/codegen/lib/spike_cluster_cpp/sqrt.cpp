
// Include Files
#include "sqrt.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : emxArray_real_T *x
// Return Type  : void
//
void b_sqrt(emxArray_real_T *x) {
  int nx;
  int k;
  nx = x->size[1];
  for (k = 1; k <= nx; k++) {
    x->data[k - 1] = 1.0;
  }
}

//
// Arguments    : emxArray_real_T *x
// Return Type  : void
//
void c_sqrt(emxArray_real_T *x) {
  int nx;
  int k;
  nx = x->size[0];
  for (k = 0; k + 1 <= nx; k++) {
    x->data[k] = std::sqrt(x->data[k]);
  }
}
