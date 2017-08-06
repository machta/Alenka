
// Include Files
#include "sort1.h"
#include "rt_nonfinite.h"
#include "sortIdx.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"

// Type Definitions
#ifndef struct_emxArray_int32_T_1
#define struct_emxArray_int32_T_1

struct emxArray_int32_T_1 {
  int data[1];
  int size[1];
};

#endif // struct_emxArray_int32_T_1

// Function Declarations
static void b_sort(emxArray_real_T *x, emxArray_int32_T *idx);
static void e_sort(emxArray_real_T *x, int dim, emxArray_int32_T *idx);
static void g_sort(emxArray_real_T *x, int dim, emxArray_int32_T *idx);

// Function Definitions

//
// Arguments    : emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
static void b_sort(emxArray_real_T *x, emxArray_int32_T *idx) {
  emxArray_real_T *vwork;
  int i5;
  int x_idx_0;
  int i6;
  emxArray_int32_T *iidx;
  emxInit_real_T1(&vwork, 1);
  i5 = x->size[1];
  x_idx_0 = x->size[1];
  i6 = vwork->size[0];
  vwork->size[0] = x_idx_0;
  emxEnsureCapacity((emxArray__common *)vwork, i6, (int)sizeof(double));
  i6 = idx->size[0] * idx->size[1];
  idx->size[0] = 1;
  idx->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)idx, i6, (int)sizeof(int));
  for (x_idx_0 = 0; x_idx_0 + 1 <= i5; x_idx_0++) {
    vwork->data[x_idx_0] = x->data[x_idx_0];
  }

  emxInit_int32_T(&iidx, 1);
  b_sortIdx(vwork, iidx);
  for (x_idx_0 = 0; x_idx_0 + 1 <= i5; x_idx_0++) {
    x->data[x_idx_0] = vwork->data[x_idx_0];
    idx->data[x_idx_0] = iidx->data[x_idx_0];
  }

  emxFree_int32_T(&iidx);
  emxFree_real_T(&vwork);
}

//
// Arguments    : emxArray_real_T *x
//                int dim
//                emxArray_int32_T *idx
// Return Type  : void
//
static void e_sort(emxArray_real_T *x, int dim, emxArray_int32_T *idx) {
  emxArray_real_T *vwork;
  int i9;
  int vstride;
  int k;
  int iv1[2];
  int npages;
  int pagesize;
  int i;
  emxArray_int32_T *iidx;
  int pageoffset;
  int j;
  int idx0;
  emxInit_real_T1(&vwork, 1);
  i9 = x->size[dim - 1];
  vstride = x->size[dim - 1];
  k = vwork->size[0];
  vwork->size[0] = vstride;
  emxEnsureCapacity((emxArray__common *)vwork, k, (int)sizeof(double));
  for (k = 0; k < 2; k++) {
    iv1[k] = x->size[k];
  }

  k = idx->size[0] * idx->size[1];
  idx->size[0] = iv1[0];
  idx->size[1] = iv1[1];
  emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
  vstride = 1;
  k = 1;
  while (k <= dim - 1) {
    vstride *= x->size[0];
    k = 2;
  }

  npages = 1;
  k = dim + 1;
  while (k < 3) {
    npages *= x->size[1];
    k = 3;
  }

  pagesize = x->size[dim - 1] * vstride;
  i = 1;
  emxInit_int32_T(&iidx, 1);
  while (i <= npages) {
    pageoffset = (i - 1) * pagesize;
    for (j = 0; j + 1 <= vstride; j++) {
      idx0 = pageoffset + j;
      for (k = 0; k + 1 <= i9; k++) {
        vwork->data[k] = x->data[idx0 + k * vstride];
      }

      b_sortIdx(vwork, iidx);
      for (k = 0; k + 1 <= i9; k++) {
        x->data[idx0 + k * vstride] = vwork->data[k];
        idx->data[idx0 + k * vstride] = iidx->data[k];
      }
    }

    i++;
  }

  emxFree_int32_T(&iidx);
  emxFree_real_T(&vwork);
}

//
// Arguments    : emxArray_real_T *x
//                int dim
//                emxArray_int32_T *idx
// Return Type  : void
//
static void g_sort(emxArray_real_T *x, int dim, emxArray_int32_T *idx) {
  int i12;
  emxArray_real_T *vwork;
  int vstride;
  int x_idx_0;
  int j;
  emxArray_int32_T *iidx;
  if (dim <= 1) {
    i12 = x->size[0];
  } else {
    i12 = 1;
  }

  emxInit_real_T1(&vwork, 1);
  vstride = vwork->size[0];
  vwork->size[0] = i12;
  emxEnsureCapacity((emxArray__common *)vwork, vstride, (int)sizeof(double));
  x_idx_0 = x->size[0];
  vstride = idx->size[0];
  idx->size[0] = x_idx_0;
  emxEnsureCapacity((emxArray__common *)idx, vstride, (int)sizeof(int));
  vstride = 1;
  x_idx_0 = 1;
  while (x_idx_0 <= dim - 1) {
    vstride *= x->size[0];
    x_idx_0 = 2;
  }

  j = 0;
  emxInit_int32_T(&iidx, 1);
  while (j + 1 <= vstride) {
    for (x_idx_0 = 0; x_idx_0 + 1 <= i12; x_idx_0++) {
      vwork->data[x_idx_0] = x->data[j + x_idx_0 * vstride];
    }

    b_sortIdx(vwork, iidx);
    for (x_idx_0 = 0; x_idx_0 + 1 <= i12; x_idx_0++) {
      x->data[j + x_idx_0 * vstride] = vwork->data[x_idx_0];
      idx->data[j + x_idx_0 * vstride] = iidx->data[x_idx_0];
    }

    j++;
  }

  emxFree_int32_T(&iidx);
  emxFree_real_T(&vwork);
}

//
// Arguments    : double x_data[]
//                int x_size[2]
// Return Type  : void
//
void c_sort(double x_data[], int x_size[2]) {
  int i7;
  double vwork_data[1];
  int vwork_size[1];
  int k;
  emxArray_int32_T_1 b_vwork_data;
  i7 = x_size[1];
  vwork_size[0] = (signed char)x_size[1];
  k = 1;
  while (k <= i7) {
    vwork_data[0] = x_data[0];
    k = 2;
  }

  c_sortIdx(vwork_data, vwork_size, b_vwork_data.data, b_vwork_data.size);
  k = 1;
  while (k <= i7) {
    x_data[0] = vwork_data[0];
    k = 2;
  }
}

//
// Arguments    : emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
void d_sort(emxArray_real_T *x, emxArray_int32_T *idx) {
  int dim;
  dim = 2;
  if (x->size[0] != 1) {
    dim = 1;
  }

  e_sort(x, dim, idx);
}

//
// Arguments    : emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
void f_sort(emxArray_real_T *x, emxArray_int32_T *idx) {
  int dim;
  dim = 2;
  if (x->size[0] != 1) {
    dim = 1;
  }

  g_sort(x, dim, idx);
}

//
// Arguments    : emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
void sort(emxArray_real_T *x, emxArray_int32_T *idx) { b_sort(x, idx); }
