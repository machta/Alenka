
// Include Files
#include "sum.h"
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
void b_sum(const emxArray_real_T *x, emxArray_real_T *y) {
  combine_vector_elements(x, y);
}

//
// Arguments    : const emxArray_boolean_T *x
//                emxArray_real_T *y
// Return Type  : void
//
void c_sum(const emxArray_boolean_T *x, emxArray_real_T *y) {
  int vlen;
  int i;
  int xoffset;
  double s;
  int k;
  vlen = y->size[0] * y->size[1];
  y->size[0] = 1;
  y->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)y, vlen, (int)sizeof(double));
  if ((x->size[0] == 0) || (x->size[1] == 0)) {
    vlen = y->size[0] * y->size[1];
    y->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)y, vlen, (int)sizeof(double));
    i = y->size[1];
    for (vlen = 0; vlen < i; vlen++) {
      y->data[y->size[0] * vlen] = 0.0;
    }
  } else {
    vlen = x->size[0];
    for (i = 0; i + 1 <= x->size[1]; i++) {
      xoffset = i * vlen;
      s = x->data[xoffset];
      for (k = 2; k <= vlen; k++) {
        s += (double)x->data[(xoffset + k) - 1];
      }

      y->data[i] = s;
    }
  }
}

//
// Arguments    : const emxArray_real_T *x
// Return Type  : double
//
double d_sum(const emxArray_real_T *x) {
  double y;
  int k;
  if (x->size[0] == 0) {
    y = 0.0;
  } else {
    y = x->data[0];
    for (k = 2; k <= x->size[0]; k++) {
      y += x->data[k - 1];
    }
  }

  return y;
}

//
// Arguments    : const emxArray_boolean_T *x
// Return Type  : double
//
double sum(const emxArray_boolean_T *x) {
  double y;
  int k;
  if (x->size[0] == 0) {
    y = 0.0;
  } else {
    y = x->data[0];
    for (k = 2; k <= x->size[0]; k++) {
      y += (double)x->data[k - 1];
    }
  }

  return y;
}
