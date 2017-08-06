
// Include Files
#include "xscal.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : int n
//                double a
//                emxArray_real_T *x
//                int ix0
// Return Type  : void
//
void xscal(int n, double a, emxArray_real_T *x, int ix0) {
  int i8;
  int k;
  i8 = (ix0 + n) - 1;
  for (k = ix0; k <= i8; k++) {
    x->data[k - 1] *= a;
  }
}
