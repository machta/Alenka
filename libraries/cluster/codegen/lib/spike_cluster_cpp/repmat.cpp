
// Include Files
#include "repmat.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *a
//                double varargin_1
//                emxArray_real_T *b
// Return Type  : void
//
void repmat(const emxArray_real_T *a, double varargin_1, emxArray_real_T *b) {
  int outsize_idx_1;
  int ibmat;
  boolean_T p;
  int itilerow;
  outsize_idx_1 = a->size[1];
  ibmat = b->size[0] * b->size[1];
  b->size[0] = (int)varargin_1;
  b->size[1] = outsize_idx_1;
  emxEnsureCapacity((emxArray__common *)b, ibmat, (int)sizeof(double));
  if (!(a->size[1] == 0)) {
    if ((int)varargin_1 == 0) {
      p = true;
    } else if (outsize_idx_1 == 0) {
      p = true;
    } else {
      p = false;
    }

    if (!p) {
      for (outsize_idx_1 = 0; outsize_idx_1 + 1 <= a->size[1];
           outsize_idx_1++) {
        ibmat = outsize_idx_1 * (int)varargin_1;
        for (itilerow = 1; itilerow <= (int)varargin_1; itilerow++) {
          b->data[(ibmat + itilerow) - 1] = a->data[outsize_idx_1];
        }
      }
    }
  }
}
