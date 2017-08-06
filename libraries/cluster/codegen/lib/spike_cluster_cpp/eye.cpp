
// Include Files
#include "eye.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"

// Function Definitions

//
// Arguments    : const double varargin_1[2]
//                emxArray_real_T *I
// Return Type  : void
//
void eye(const double varargin_1[2], emxArray_real_T *I) {
  double minval;
  int k;
  int loop_ub;
  if ((varargin_1[0] <= varargin_1[1]) || rtIsNaN(varargin_1[1])) {
    minval = varargin_1[0];
  } else {
    minval = varargin_1[1];
  }

  k = I->size[0] * I->size[1];
  I->size[0] = (int)varargin_1[0];
  I->size[1] = (int)varargin_1[1];
  emxEnsureCapacity((emxArray__common *)I, k, (int)sizeof(double));
  loop_ub = (int)varargin_1[0] * (int)varargin_1[1];
  for (k = 0; k < loop_ub; k++) {
    I->data[k] = 0.0;
  }

  if ((int)minval > 0) {
    for (k = 0; k + 1 <= (int)minval; k++) {
      I->data[k + I->size[0] * k] = 1.0;
    }
  }
}
