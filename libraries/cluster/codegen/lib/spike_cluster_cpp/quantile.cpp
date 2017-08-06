
// Include Files
#include "quantile.h"
#include "mod.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_rtwutil.h"

// Function Definitions

//
// Arguments    : const double x[100]
// Return Type  : double
//
double quantile(const double x[100]) {
  double y;
  int iwork[100];
  int idx[100];
  int nj;
  int i;
  boolean_T p;
  int i2;
  int j;
  int pEnd;
  double r;
  int b_p;
  int q;
  int qEnd;
  int kEnd;
  for (nj = 0; nj <= 99; nj += 2) {
    if ((x[nj] <= x[nj + 1]) || rtIsNaN(x[nj + 1])) {
      p = true;
    } else {
      p = false;
    }

    if (p) {
      idx[nj] = nj + 1;
      idx[nj + 1] = nj + 2;
    } else {
      idx[nj] = nj + 2;
      idx[nj + 1] = nj + 1;
    }
  }

  i = 2;
  while (i < 100) {
    i2 = i << 1;
    j = 1;
    for (pEnd = 1 + i; pEnd < 101; pEnd = qEnd + i) {
      b_p = j;
      q = pEnd - 1;
      qEnd = j + i2;
      if (qEnd > 101) {
        qEnd = 101;
      }

      nj = 0;
      kEnd = qEnd - j;
      while (nj + 1 <= kEnd) {
        if ((x[idx[b_p - 1] - 1] <= x[idx[q] - 1]) || rtIsNaN(x[idx[q] - 1])) {
          p = true;
        } else {
          p = false;
        }

        if (p) {
          iwork[nj] = idx[b_p - 1];
          b_p++;
          if (b_p == pEnd) {
            while (q + 1 < qEnd) {
              nj++;
              iwork[nj] = idx[q];
              q++;
            }
          }
        } else {
          iwork[nj] = idx[q];
          q++;
          if (q + 1 == qEnd) {
            while (b_p < pEnd) {
              nj++;
              iwork[nj] = idx[b_p - 1];
              b_p++;
            }
          }
        }

        nj++;
      }

      for (nj = 0; nj + 1 <= kEnd; nj++) {
        idx[(j + nj) - 1] = iwork[nj];
      }

      j = qEnd;
    }

    i = i2;
  }

  nj = 100;
  while ((nj > 0) && rtIsNaN(x[idx[nj - 1] - 1])) {
    nj--;
  }

  if (nj < 1) {
    y = rtNaN;
  } else if (nj == 1) {
    y = x[idx[0] - 1];
  } else {
    r = 0.95 * (double)nj;
    i = (int)rt_roundd_snf(r);
    if (nj <= i) {
      y = x[idx[nj - 1] - 1];
    } else {
      r -= (double)i;
      y = (0.5 - r) * x[idx[i - 1] - 1] + (0.5 + r) * x[idx[i] - 1];
    }
  }

  return y;
}
