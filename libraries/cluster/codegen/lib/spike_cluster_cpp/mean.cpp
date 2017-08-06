
// Include Files
#include "mean.h"
#include "combine_vector_elements.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *x
//                emxArray_real_T *y
// Return Type  : void
//
void mean(const emxArray_real_T *x, emxArray_real_T *y) {
  int b_y;
  int c_y;
  int b_x;
  combine_vector_elements(x, y);
  b_y = y->size[0] * y->size[1];
  y->size[0] = 1;
  emxEnsureCapacity((emxArray__common *)y, b_y, (int)sizeof(double));
  b_y = y->size[0];
  c_y = y->size[1];
  b_x = x->size[0];
  c_y *= b_y;
  for (b_y = 0; b_y < c_y; b_y++) {
    y->data[b_y] /= (double)b_x;
  }
}
