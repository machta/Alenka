
// Include Files
#include "xswap.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : int n
//                emxArray_real_T *x
//                int ix0
//                int iy0
// Return Type  : void
//
void xswap(int n, emxArray_real_T *x, int ix0, int iy0) {
  int ix;
  int iy;
  int k;
  double temp;
  ix = ix0 - 1;
  iy = iy0 - 1;
  for (k = 1; k <= n; k++) {
    temp = x->data[ix];
    x->data[ix] = x->data[iy];
    x->data[iy] = temp;
    ix++;
    iy++;
  }
}
