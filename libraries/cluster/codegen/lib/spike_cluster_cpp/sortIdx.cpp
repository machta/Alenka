
// Include Files
#include "sortIdx.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"

// Function Declarations
static void b_merge(int idx_data[], double x_data[], int np, int nq,
                    int iwork_data[], double xwork_data[]);
static void merge(emxArray_int32_T *idx, emxArray_real_T *x, int offset, int np,
                  int nq, emxArray_int32_T *iwork, emxArray_real_T *xwork);
static void merge_block(emxArray_int32_T *idx, emxArray_real_T *x, int offset,
                        int n, int preSortLevel, emxArray_int32_T *iwork,
                        emxArray_real_T *xwork);

// Function Definitions

//
// Arguments    : int idx_data[]
//                double x_data[]
//                int np
//                int nq
//                int iwork_data[]
//                double xwork_data[]
// Return Type  : void
//
static void b_merge(int idx_data[], double x_data[], int np, int nq,
                    int iwork_data[], double xwork_data[]) {
  int n;
  int j;
  if (nq != 0) {
    n = np + nq;
    j = 1;
    while (j <= n) {
      iwork_data[0] = idx_data[0];
      xwork_data[0] = x_data[0];
      j = 2;
    }

    n = np + nq;
    do {
      idx_data[0] = iwork_data[0];
      x_data[0] = xwork_data[0];
    } while (1 < n);

    j = 1;
    while (j <= np) {
      idx_data[0] = iwork_data[0];
      x_data[1] = xwork_data[0];
      j = 2;
    }
  }
}

//
// Arguments    : emxArray_int32_T *idx
//                emxArray_real_T *x
//                int offset
//                int np
//                int nq
//                emxArray_int32_T *iwork
//                emxArray_real_T *xwork
// Return Type  : void
//
static void merge(emxArray_int32_T *idx, emxArray_real_T *x, int offset, int np,
                  int nq, emxArray_int32_T *iwork, emxArray_real_T *xwork) {
  int n;
  int qend;
  int p;
  int iout;
  int exitg1;
  if (nq != 0) {
    n = np + nq;
    for (qend = 0; qend + 1 <= n; qend++) {
      iwork->data[qend] = idx->data[offset + qend];
      xwork->data[qend] = x->data[offset + qend];
    }

    p = 0;
    n = np;
    qend = np + nq;
    iout = offset - 1;
    do {
      exitg1 = 0;
      iout++;
      if (xwork->data[p] >= xwork->data[n]) {
        idx->data[iout] = iwork->data[p];
        x->data[iout] = xwork->data[p];
        if (p + 1 < np) {
          p++;
        } else {
          exitg1 = 1;
        }
      } else {
        idx->data[iout] = iwork->data[n];
        x->data[iout] = xwork->data[n];
        if (n + 1 < qend) {
          n++;
        } else {
          n = (iout - p) + 1;
          while (p + 1 <= np) {
            idx->data[n + p] = iwork->data[p];
            x->data[n + p] = xwork->data[p];
            p++;
          }

          exitg1 = 1;
        }
      }
    } while (exitg1 == 0);
  }
}

//
// Arguments    : emxArray_int32_T *idx
//                emxArray_real_T *x
//                int offset
//                int n
//                int preSortLevel
//                emxArray_int32_T *iwork
//                emxArray_real_T *xwork
// Return Type  : void
//
static void merge_block(emxArray_int32_T *idx, emxArray_real_T *x, int offset,
                        int n, int preSortLevel, emxArray_int32_T *iwork,
                        emxArray_real_T *xwork) {
  int nPairs;
  int bLen;
  int tailOffset;
  int nTail;
  nPairs = n >> preSortLevel;
  bLen = 1 << preSortLevel;
  while (nPairs > 1) {
    if ((nPairs & 1) != 0) {
      nPairs--;
      tailOffset = bLen * nPairs;
      nTail = n - tailOffset;
      if (nTail > bLen) {
        merge(idx, x, offset + tailOffset, bLen, nTail - bLen, iwork, xwork);
      }
    }

    tailOffset = bLen << 1;
    nPairs >>= 1;
    for (nTail = 1; nTail <= nPairs; nTail++) {
      merge(idx, x, offset + (nTail - 1) * tailOffset, bLen, bLen, iwork,
            xwork);
    }

    bLen = tailOffset;
  }

  if (n > bLen) {
    merge(idx, x, offset, bLen, n - bLen, iwork, xwork);
  }
}

//
// Arguments    : emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
void b_sortIdx(emxArray_real_T *x, emxArray_int32_T *idx) {
  emxArray_real_T *b_x;
  int ib;
  int wOffset;
  int i;
  int n;
  double x4[4];
  int idx4[4];
  emxArray_int32_T *iwork;
  emxArray_real_T *xwork;
  int nNaNs;
  int k;
  signed char perm[4];
  int nNonNaN;
  int i3;
  int i4;
  int nBlocks;
  int b_iwork[256];
  double b_xwork[256];
  int bLen;
  int bLen2;
  int nPairs;
  int exitg1;
  emxInit_real_T1(&b_x, 1);
  ib = x->size[0];
  wOffset = b_x->size[0];
  b_x->size[0] = x->size[0];
  emxEnsureCapacity((emxArray__common *)b_x, wOffset, (int)sizeof(double));
  i = x->size[0];
  for (wOffset = 0; wOffset < i; wOffset++) {
    b_x->data[wOffset] = x->data[wOffset];
  }

  wOffset = idx->size[0];
  idx->size[0] = ib;
  emxEnsureCapacity((emxArray__common *)idx, wOffset, (int)sizeof(int));
  for (wOffset = 0; wOffset < ib; wOffset++) {
    idx->data[wOffset] = 0;
  }

  n = x->size[0];
  for (i = 0; i < 4; i++) {
    x4[i] = 0.0;
    idx4[i] = 0;
  }

  emxInit_int32_T(&iwork, 1);
  wOffset = iwork->size[0];
  iwork->size[0] = ib;
  emxEnsureCapacity((emxArray__common *)iwork, wOffset, (int)sizeof(int));
  i = iwork->size[0];
  wOffset = iwork->size[0];
  iwork->size[0] = i;
  emxEnsureCapacity((emxArray__common *)iwork, wOffset, (int)sizeof(int));
  for (wOffset = 0; wOffset < i; wOffset++) {
    iwork->data[wOffset] = 0;
  }

  emxInit_real_T1(&xwork, 1);
  i = x->size[0];
  wOffset = xwork->size[0];
  xwork->size[0] = i;
  emxEnsureCapacity((emxArray__common *)xwork, wOffset, (int)sizeof(double));
  i = xwork->size[0];
  wOffset = xwork->size[0];
  xwork->size[0] = i;
  emxEnsureCapacity((emxArray__common *)xwork, wOffset, (int)sizeof(double));
  for (wOffset = 0; wOffset < i; wOffset++) {
    xwork->data[wOffset] = 0.0;
  }

  nNaNs = 0;
  ib = 0;
  for (k = 0; k + 1 <= n; k++) {
    if (rtIsNaN(b_x->data[k])) {
      idx->data[(n - nNaNs) - 1] = k + 1;
      xwork->data[(n - nNaNs) - 1] = b_x->data[k];
      nNaNs++;
    } else {
      ib++;
      idx4[ib - 1] = k + 1;
      x4[ib - 1] = b_x->data[k];
      if (ib == 4) {
        i = k - nNaNs;
        if (x4[0] >= x4[1]) {
          ib = 1;
          wOffset = 2;
        } else {
          ib = 2;
          wOffset = 1;
        }

        if (x4[2] >= x4[3]) {
          i3 = 3;
          i4 = 4;
        } else {
          i3 = 4;
          i4 = 3;
        }

        if (x4[ib - 1] >= x4[i3 - 1]) {
          if (x4[wOffset - 1] >= x4[i3 - 1]) {
            perm[0] = (signed char)ib;
            perm[1] = (signed char)wOffset;
            perm[2] = (signed char)i3;
            perm[3] = (signed char)i4;
          } else if (x4[wOffset - 1] >= x4[i4 - 1]) {
            perm[0] = (signed char)ib;
            perm[1] = (signed char)i3;
            perm[2] = (signed char)wOffset;
            perm[3] = (signed char)i4;
          } else {
            perm[0] = (signed char)ib;
            perm[1] = (signed char)i3;
            perm[2] = (signed char)i4;
            perm[3] = (signed char)wOffset;
          }
        } else if (x4[ib - 1] >= x4[i4 - 1]) {
          if (x4[wOffset - 1] >= x4[i4 - 1]) {
            perm[0] = (signed char)i3;
            perm[1] = (signed char)ib;
            perm[2] = (signed char)wOffset;
            perm[3] = (signed char)i4;
          } else {
            perm[0] = (signed char)i3;
            perm[1] = (signed char)ib;
            perm[2] = (signed char)i4;
            perm[3] = (signed char)wOffset;
          }
        } else {
          perm[0] = (signed char)i3;
          perm[1] = (signed char)i4;
          perm[2] = (signed char)ib;
          perm[3] = (signed char)wOffset;
        }

        idx->data[i - 3] = idx4[perm[0] - 1];
        idx->data[i - 2] = idx4[perm[1] - 1];
        idx->data[i - 1] = idx4[perm[2] - 1];
        idx->data[i] = idx4[perm[3] - 1];
        b_x->data[i - 3] = x4[perm[0] - 1];
        b_x->data[i - 2] = x4[perm[1] - 1];
        b_x->data[i - 1] = x4[perm[2] - 1];
        b_x->data[i] = x4[perm[3] - 1];
        ib = 0;
      }
    }
  }

  wOffset = (x->size[0] - nNaNs) - 1;
  if (ib > 0) {
    for (i = 0; i < 4; i++) {
      perm[i] = 0;
    }

    if (ib == 1) {
      perm[0] = 1;
    } else if (ib == 2) {
      if (x4[0] >= x4[1]) {
        perm[0] = 1;
        perm[1] = 2;
      } else {
        perm[0] = 2;
        perm[1] = 1;
      }
    } else if (x4[0] >= x4[1]) {
      if (x4[1] >= x4[2]) {
        perm[0] = 1;
        perm[1] = 2;
        perm[2] = 3;
      } else if (x4[0] >= x4[2]) {
        perm[0] = 1;
        perm[1] = 3;
        perm[2] = 2;
      } else {
        perm[0] = 3;
        perm[1] = 1;
        perm[2] = 2;
      }
    } else if (x4[0] >= x4[2]) {
      perm[0] = 2;
      perm[1] = 1;
      perm[2] = 3;
    } else if (x4[1] >= x4[2]) {
      perm[0] = 2;
      perm[1] = 3;
      perm[2] = 1;
    } else {
      perm[0] = 3;
      perm[1] = 2;
      perm[2] = 1;
    }

    for (k = 1; k <= ib; k++) {
      idx->data[(wOffset - ib) + k] = idx4[perm[k - 1] - 1];
      b_x->data[(wOffset - ib) + k] = x4[perm[k - 1] - 1];
    }
  }

  i = nNaNs >> 1;
  for (k = 1; k <= i; k++) {
    ib = idx->data[wOffset + k];
    idx->data[wOffset + k] = idx->data[n - k];
    idx->data[n - k] = ib;
    b_x->data[wOffset + k] = xwork->data[n - k];
    b_x->data[n - k] = xwork->data[wOffset + k];
  }

  if ((nNaNs & 1) != 0) {
    b_x->data[(wOffset + i) + 1] = xwork->data[(wOffset + i) + 1];
  }

  nNonNaN = x->size[0] - nNaNs;
  i = 2;
  if (nNonNaN > 1) {
    if (x->size[0] >= 256) {
      nBlocks = nNonNaN >> 8;
      if (nBlocks > 0) {
        for (i3 = 1; i3 <= nBlocks; i3++) {
          i4 = ((i3 - 1) << 8) - 1;
          for (n = 0; n < 6; n++) {
            bLen = 1 << (n + 2);
            bLen2 = bLen << 1;
            nPairs = 256 >> (n + 3);
            for (k = 1; k <= nPairs; k++) {
              ib = i4 + (k - 1) * bLen2;
              for (i = 1; i <= bLen2; i++) {
                b_iwork[i - 1] = idx->data[ib + i];
                b_xwork[i - 1] = b_x->data[ib + i];
              }

              wOffset = 0;
              i = bLen;
              do {
                exitg1 = 0;
                ib++;
                if (b_xwork[wOffset] >= b_xwork[i]) {
                  idx->data[ib] = b_iwork[wOffset];
                  b_x->data[ib] = b_xwork[wOffset];
                  if (wOffset + 1 < bLen) {
                    wOffset++;
                  } else {
                    exitg1 = 1;
                  }
                } else {
                  idx->data[ib] = b_iwork[i];
                  b_x->data[ib] = b_xwork[i];
                  if (i + 1 < bLen2) {
                    i++;
                  } else {
                    i = ib - wOffset;
                    while (wOffset + 1 <= bLen) {
                      idx->data[(i + wOffset) + 1] = b_iwork[wOffset];
                      b_x->data[(i + wOffset) + 1] = b_xwork[wOffset];
                      wOffset++;
                    }

                    exitg1 = 1;
                  }
                }
              } while (exitg1 == 0);
            }
          }
        }

        i = nBlocks << 8;
        ib = nNonNaN - i;
        if (ib > 0) {
          merge_block(idx, b_x, i, ib, 2, iwork, xwork);
        }

        i = 8;
      }
    }

    merge_block(idx, b_x, 0, nNonNaN, i, iwork, xwork);
  }

  if ((nNaNs > 0) && (nNonNaN > 0)) {
    for (k = 0; k + 1 <= nNaNs; k++) {
      xwork->data[k] = b_x->data[nNonNaN + k];
      iwork->data[k] = idx->data[nNonNaN + k];
    }

    for (k = nNonNaN - 1; k + 1 > 0; k--) {
      b_x->data[nNaNs + k] = b_x->data[k];
      idx->data[nNaNs + k] = idx->data[k];
    }

    for (k = 0; k + 1 <= nNaNs; k++) {
      b_x->data[k] = xwork->data[k];
      idx->data[k] = iwork->data[k];
    }
  }

  emxFree_real_T(&xwork);
  emxFree_int32_T(&iwork);
  wOffset = x->size[0];
  x->size[0] = b_x->size[0];
  emxEnsureCapacity((emxArray__common *)x, wOffset, (int)sizeof(double));
  i = b_x->size[0];
  for (wOffset = 0; wOffset < i; wOffset++) {
    x->data[wOffset] = b_x->data[wOffset];
  }

  emxFree_real_T(&b_x);
}

//
// Arguments    : double x_data[]
//                int x_size[1]
//                int idx_data[]
//                int idx_size[1]
// Return Type  : void
//
void c_sortIdx(double x_data[], int x_size[1], int idx_data[],
               int idx_size[1]) {
  signed char unnamed_idx_0;
  int x_size_idx_0;
  int i;
  int ib;
  double b_x_data[1];
  int n;
  double x4[4];
  signed char idx4[4];
  emxArray_int32_T *iwork;
  emxArray_real_T *xwork;
  int nNaNs;
  double xwork_data[1];
  int k;
  signed char perm[4];
  int nNonNaN;
  int nPairs;
  int iwork_data[1];
  unnamed_idx_0 = (signed char)x_size[0];
  x_size_idx_0 = x_size[0];
  i = x_size[0];
  for (ib = 0; ib < i; ib++) {
    b_x_data[ib] = x_data[ib];
  }

  idx_size[0] = unnamed_idx_0;
  i = unnamed_idx_0;
  for (ib = 0; ib < i; ib++) {
    idx_data[ib] = 0;
  }

  n = x_size[0];
  for (i = 0; i < 4; i++) {
    x4[i] = 0.0;
    idx4[i] = 0;
  }

  emxInit_int32_T(&iwork, 1);
  emxInit_real_T1(&xwork, 1);
  ib = iwork->size[0];
  iwork->size[0] = unnamed_idx_0;
  emxEnsureCapacity((emxArray__common *)iwork, ib, (int)sizeof(int));
  ib = xwork->size[0];
  xwork->size[0] = (signed char)x_size[0];
  emxEnsureCapacity((emxArray__common *)xwork, ib, (int)sizeof(double));
  i = xwork->size[0];
  emxFree_real_T(&xwork);
  for (ib = 0; ib < i; ib++) {
    xwork_data[ib] = 0.0;
  }

  nNaNs = 0;
  ib = 0;
  k = 1;
  while (k <= n) {
    if (rtIsNaN(b_x_data[0])) {
      idx_data[0] = 1;
      xwork_data[0] = b_x_data[0];
      nNaNs++;
    } else {
      ib++;
      idx4[ib - 1] = 1;
      x4[ib - 1] = b_x_data[0];
      if (ib == 4) {
        if (x4[0] <= x4[1]) {
          i = 0;
          ib = 2;
        } else {
          i = 1;
          ib = 1;
        }

        if (x4[2] <= x4[3]) {
          nPairs = 2;
          nNonNaN = 4;
        } else {
          nPairs = 3;
          nNonNaN = 3;
        }

        if (x4[i] <= x4[nPairs]) {
          if (x4[ib - 1] <= x4[nPairs]) {
            perm[3] = (signed char)nNonNaN;
          } else if (x4[ib - 1] <= x4[nNonNaN - 1]) {
            perm[3] = (signed char)nNonNaN;
          } else {
            perm[3] = (signed char)ib;
          }
        } else if (x4[i] <= x4[nNonNaN - 1]) {
          if (x4[ib - 1] <= x4[nNonNaN - 1]) {
            perm[3] = (signed char)nNonNaN;
          } else {
            perm[3] = (signed char)ib;
          }
        } else {
          perm[3] = (signed char)ib;
        }

        idx_data[0] = idx4[perm[3] - 1];
        b_x_data[0] = x4[perm[3] - 1];
        ib = 0;
      }
    }

    k = 2;
  }

  if (ib > 0) {
    for (i = 0; i < 4; i++) {
      perm[i] = 0;
    }

    if (ib == 1) {
      perm[0] = 1;
    } else if (ib == 2) {
      if (x4[0] <= x4[1]) {
        perm[0] = 1;
        perm[1] = 2;
      } else {
        perm[0] = 2;
        perm[1] = 1;
      }
    } else if (x4[0] <= x4[1]) {
      if (x4[1] <= x4[2]) {
        perm[0] = 1;
        perm[1] = 2;
        perm[2] = 3;
      } else if (x4[0] <= x4[2]) {
        perm[0] = 1;
        perm[1] = 3;
        perm[2] = 2;
      } else {
        perm[0] = 3;
        perm[1] = 1;
        perm[2] = 2;
      }
    } else if (x4[0] <= x4[2]) {
      perm[0] = 2;
      perm[1] = 1;
      perm[2] = 3;
    } else if (x4[1] <= x4[2]) {
      perm[0] = 2;
      perm[1] = 3;
      perm[2] = 1;
    } else {
      perm[0] = 3;
      perm[1] = 2;
      perm[2] = 1;
    }

    for (k = 0; k + 1 <= ib; k++) {
      idx_data[0] = idx4[perm[k] - 1];
      b_x_data[0] = x4[perm[k] - 1];
    }
  }

  i = nNaNs >> 1;
  for (k = 1; k <= i; k++) {
    b_x_data[0] = xwork_data[0];
  }

  if ((nNaNs & 1) != 0) {
    b_x_data[0] = xwork_data[0];
  }

  nNonNaN = x_size[0] - nNaNs;
  if (nNonNaN > 1) {
    i = iwork->size[0];
    for (ib = 0; ib < i; ib++) {
      iwork_data[ib] = 0;
    }

    nPairs = nNonNaN >> 2;
    i = 4;
    while (nPairs > 1) {
      if ((nPairs & 1) != 0) {
        nPairs--;
        ib = nNonNaN - i * nPairs;
        if (ib > i) {
          b_merge(idx_data, b_x_data, i, ib - i, iwork_data, xwork_data);
        }
      }

      nPairs >>= 1;
      for (k = 1; k <= nPairs; k++) {
        b_merge(idx_data, b_x_data, i, i, iwork_data, xwork_data);
      }

      i <<= 1;
    }

    if (nNonNaN > i) {
      b_merge(idx_data, b_x_data, i, nNonNaN - i, iwork_data, xwork_data);
    }
  }

  emxFree_int32_T(&iwork);
  x_size[0] = x_size_idx_0;
  for (ib = 0; ib < x_size_idx_0; ib++) {
    x_data[ib] = b_x_data[ib];
  }
}

//
// Arguments    : const emxArray_real_T *x
//                emxArray_int32_T *idx
// Return Type  : void
//
void sortIdx(const emxArray_real_T *x, emxArray_int32_T *idx) {
  int n;
  unsigned int unnamed_idx_0;
  int k;
  int i;
  emxArray_int32_T *iwork;
  boolean_T p;
  int i2;
  int j;
  int pEnd;
  int b_p;
  int q;
  int qEnd;
  int kEnd;
  n = x->size[0] + 1;
  unnamed_idx_0 = (unsigned int)x->size[0];
  k = idx->size[0];
  idx->size[0] = (int)unnamed_idx_0;
  emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
  i = (int)unnamed_idx_0;
  for (k = 0; k < i; k++) {
    idx->data[k] = 0;
  }

  if (x->size[0] != 0) {
    emxInit_int32_T(&iwork, 1);
    k = iwork->size[0];
    iwork->size[0] = (int)unnamed_idx_0;
    emxEnsureCapacity((emxArray__common *)iwork, k, (int)sizeof(int));
    for (k = 1; k <= n - 2; k += 2) {
      if ((x->data[k - 1] <= x->data[k]) || rtIsNaN(x->data[k])) {
        p = true;
      } else {
        p = false;
      }

      if (p) {
        idx->data[k - 1] = k;
        idx->data[k] = k + 1;
      } else {
        idx->data[k - 1] = k + 1;
        idx->data[k] = k;
      }
    }

    if ((x->size[0] & 1) != 0) {
      idx->data[x->size[0] - 1] = x->size[0];
    }

    i = 2;
    while (i < n - 1) {
      i2 = i << 1;
      j = 1;
      for (pEnd = 1 + i; pEnd < n; pEnd = qEnd + i) {
        b_p = j;
        q = pEnd - 1;
        qEnd = j + i2;
        if (qEnd > n) {
          qEnd = n;
        }

        k = 0;
        kEnd = qEnd - j;
        while (k + 1 <= kEnd) {
          if ((x->data[idx->data[b_p - 1] - 1] <= x->data[idx->data[q] - 1]) ||
              rtIsNaN(x->data[idx->data[q] - 1])) {
            p = true;
          } else {
            p = false;
          }

          if (p) {
            iwork->data[k] = idx->data[b_p - 1];
            b_p++;
            if (b_p == pEnd) {
              while (q + 1 < qEnd) {
                k++;
                iwork->data[k] = idx->data[q];
                q++;
              }
            }
          } else {
            iwork->data[k] = idx->data[q];
            q++;
            if (q + 1 == qEnd) {
              while (b_p < pEnd) {
                k++;
                iwork->data[k] = idx->data[b_p - 1];
                b_p++;
              }
            }
          }

          k++;
        }

        for (k = 0; k + 1 <= kEnd; k++) {
          idx->data[(j + k) - 1] = iwork->data[k];
        }

        j = qEnd;
      }

      i = i2;
    }

    emxFree_int32_T(&iwork);
  }
}
