
// Include Files
#include "spike_cluster_cpp.h"
#include "eml_rand.h"
#include "eml_rand_mt19937ar_stateful.h"
#include "eye.h"
#include "fprintf.h"
#include "mean.h"
#include "nullAssignment.h"
#include "pca.h"
#include "quantile.h"
#include "randperm.h"
#include "repmat.h"
#include "rt_nonfinite.h"
#include "sort1.h"
#include "sortIdx.h"
#include "spike_cluster_cpp_emxutil.h"
#include "spike_cluster_cpp_rtwutil.h"
#include "sum.h"
#include "unique.h"

// Variable Definitions
static int threadID;

#pragma omp threadprivate(threadID)

// Function Declarations
static void cluster_merging(struct0_T *cluster, const emxArray_real_T *MW);
static void my_corr(const emxArray_real_T *x, const emxArray_real_T *y,
                    emxArray_real_T *res);
static void repeat_if_smaller(emxArray_real_T *x, double n);
static void spike_cluster_sub(emxArray_real_T *data, double pca_centering,
                              emxArray_real_T *b_class, emxArray_real_T *w,
                              emxArray_real_T *area);

// Function Definitions

//
// Arguments    : struct0_T *cluster
//                const emxArray_real_T *MW
// Return Type  : void
//
static void cluster_merging(struct0_T *cluster, const emxArray_real_T *MW) {
  emxArray_real_T *qeeg;
  emxArray_real_T *C;
  emxArray_real_T *c_unique;
  emxArray_real_T *b_class;
  emxArray_real_T *area;
  emxArray_boolean_T *r8;
  emxArray_boolean_T *x;
  emxArray_int32_T *ii;
  emxArray_int32_T *idx;
  emxArray_real_T *r9;
  emxArray_boolean_T *b_cluster;
  emxArray_real_T *b_MW;
  emxArray_boolean_T *c_cluster;
  int exitg2;
  int k;
  int nx;
  int ixstart;
  int n;
  double absxk;
  boolean_T exitg5;
  int b_idx;
  double b_C[2];
  int end;
  int nb;
  double b_x;
  int exitg1;
  int exponent;
  boolean_T exitg4;
  boolean_T p;
  boolean_T guard1 = false;
  emxArray_boolean_T *b_area;
  int exitg3;
  int i11;
  int b_exponent;
  emxInit_real_T(&qeeg, 2);
  emxInit_real_T(&C, 2);
  emxInit_real_T1(&c_unique, 1);
  emxInit_real_T1(&b_class, 1);
  emxInit_real_T1(&area, 1);
  emxInit_boolean_T(&r8, 1);
  emxInit_boolean_T1(&x, 2);
  emxInit_int32_T(&ii, 1);
  emxInit_int32_T(&idx, 1);
  emxInit_real_T(&r9, 2);
  emxInit_boolean_T(&b_cluster, 1);
  emxInit_real_T(&b_MW, 2);
  emxInit_boolean_T(&c_cluster, 1);
  do {
    exitg2 = 0;
    k = b_cluster->size[0];
    b_cluster->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)b_cluster, k, (int)sizeof(boolean_T));
    nx = cluster->b_class->size[0];
    for (k = 0; k < nx; k++) {
      b_cluster->data[k] = (cluster->b_class->data[k] > 0.0);
    }

    if (sum(b_cluster) == 0.0) {
      exitg2 = 1;
    } else {
      ixstart = 1;
      n = cluster->b_class->size[0];
      absxk = cluster->b_class->data[0];
      if (cluster->b_class->size[0] > 1) {
        if (rtIsNaN(cluster->b_class->data[0])) {
          nx = 2;
          exitg5 = false;
          while ((!exitg5) && (nx <= n)) {
            ixstart = nx;
            if (!rtIsNaN(cluster->b_class->data[nx - 1])) {
              absxk = cluster->b_class->data[nx - 1];
              exitg5 = true;
            } else {
              nx++;
            }
          }
        }

        if (ixstart < cluster->b_class->size[0]) {
          while (ixstart + 1 <= n) {
            if (cluster->b_class->data[ixstart] > absxk) {
              absxk = cluster->b_class->data[ixstart];
            }

            ixstart++;
          }
        }
      }

      k = qeeg->size[0] * qeeg->size[1];
      qeeg->size[0] = MW->size[1];
      qeeg->size[1] = (int)absxk;
      emxEnsureCapacity((emxArray__common *)qeeg, k, (int)sizeof(double));
      nx = MW->size[1] * (int)absxk;
      for (k = 0; k < nx; k++) {
        qeeg->data[k] = 0.0;
      }

      for (b_idx = 0; b_idx < (int)absxk; b_idx++) {
        k = r8->size[0];
        r8->size[0] = cluster->b_class->size[0];
        emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
        nx = cluster->b_class->size[0];
        for (k = 0; k < nx; k++) {
          r8->data[k] = (cluster->b_class->data[k] == 1.0 + (double)b_idx);
        }

        end = r8->size[0] - 1;
        ixstart = 0;
        for (n = 0; n <= end; n++) {
          if (r8->data[n]) {
            ixstart++;
          }
        }

        k = ii->size[0];
        ii->size[0] = ixstart;
        emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
        ixstart = 0;
        for (n = 0; n <= end; n++) {
          if (r8->data[n]) {
            ii->data[ixstart] = n + 1;
            ixstart++;
          }
        }

        nx = MW->size[1];
        k = b_MW->size[0] * b_MW->size[1];
        b_MW->size[0] = ii->size[0];
        b_MW->size[1] = nx;
        emxEnsureCapacity((emxArray__common *)b_MW, k, (int)sizeof(double));
        for (k = 0; k < nx; k++) {
          ixstart = ii->size[0];
          for (n = 0; n < ixstart; n++) {
            b_MW->data[n + b_MW->size[0] * k] =
                MW->data[(ii->data[n] + MW->size[0] * k) - 1];
          }
        }

        b_sum(b_MW, r9);
        k = c_unique->size[0];
        c_unique->size[0] = r9->size[1];
        emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
        nx = r9->size[1];
        for (k = 0; k < nx; k++) {
          c_unique->data[k] = r9->data[r9->size[0] * k];
        }

        nx = c_unique->size[0];
        for (k = 0; k < nx; k++) {
          qeeg->data[k + qeeg->size[0] * b_idx] = c_unique->data[k];
        }
      }

      // C=corr(qeeg);
      my_corr(qeeg, qeeg, C);
      for (k = 0; k < 2; k++) {
        b_C[k] = C->size[k];
      }

      eye(b_C, qeeg);
      end = qeeg->size[0] * qeeg->size[1];
      for (b_idx = 0; b_idx < end; b_idx++) {
        if (qeeg->data[b_idx] != 0.0) {
          C->data[b_idx] = rtMinusInf;
        }
      }

      k = x->size[0] * x->size[1];
      x->size[0] = C->size[0];
      x->size[1] = C->size[1];
      emxEnsureCapacity((emxArray__common *)x, k, (int)sizeof(boolean_T));
      nx = C->size[0] * C->size[1];
      for (k = 0; k < nx; k++) {
        x->data[k] = (C->data[k] > 0.9);
      }

      nx = x->size[0] * x->size[1];
      b_idx = 0;
      k = ii->size[0];
      ii->size[0] = nx;
      emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
      k = idx->size[0];
      idx->size[0] = nx;
      emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
      if (nx == 0) {
        k = ii->size[0];
        ii->size[0] = 0;
        emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
        k = idx->size[0];
        idx->size[0] = 0;
        emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
      } else {
        ixstart = 1;
        n = 1;
        exitg4 = false;
        while ((!exitg4) && (n <= x->size[1])) {
          guard1 = false;
          if (x->data[(ixstart + x->size[0] * (n - 1)) - 1]) {
            b_idx++;
            ii->data[b_idx - 1] = ixstart;
            idx->data[b_idx - 1] = n;
            if (b_idx >= nx) {
              exitg4 = true;
            } else {
              guard1 = true;
            }
          } else {
            guard1 = true;
          }

          if (guard1) {
            ixstart++;
            if (ixstart > x->size[0]) {
              ixstart = 1;
              n++;
            }
          }
        }

        if (nx == 1) {
          if (b_idx == 0) {
            k = ii->size[0];
            ii->size[0] = 0;
            emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
            k = idx->size[0];
            idx->size[0] = 0;
            emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
          }
        } else {
          k = ii->size[0];
          if (1 > b_idx) {
            ii->size[0] = 0;
          } else {
            ii->size[0] = b_idx;
          }

          emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
          k = idx->size[0];
          if (1 > b_idx) {
            idx->size[0] = 0;
          } else {
            idx->size[0] = b_idx;
          }

          emxEnsureCapacity((emxArray__common *)idx, k, (int)sizeof(int));
        }
      }

      k = c_unique->size[0];
      c_unique->size[0] = ii->size[0];
      emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
      nx = ii->size[0];
      for (k = 0; k < nx; k++) {
        c_unique->data[k] = ii->data[k];
      }

      k = b_class->size[0];
      b_class->size[0] = idx->size[0];
      emxEnsureCapacity((emxArray__common *)b_class, k, (int)sizeof(double));
      nx = idx->size[0];
      for (k = 0; k < nx; k++) {
        b_class->data[k] = idx->data[k];
      }

      if (c_unique->size[0] == 0) {
        exitg2 = 1;
      } else {
        //      disp('correlation merging')
        absxk = b_class->data[0];
        k = r8->size[0];
        r8->size[0] = cluster->b_class->size[0];
        emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
        nx = cluster->b_class->size[0];
        for (k = 0; k < nx; k++) {
          r8->data[k] = (cluster->b_class->data[k] == absxk);
        }

        end = r8->size[0] - 1;
        ixstart = 0;
        for (b_idx = 0; b_idx <= end; b_idx++) {
          if (r8->data[b_idx]) {
            ixstart++;
          }
        }

        k = ii->size[0];
        ii->size[0] = ixstart;
        emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
        ixstart = 0;
        for (b_idx = 0; b_idx <= end; b_idx++) {
          if (r8->data[b_idx]) {
            ii->data[ixstart] = b_idx + 1;
            ixstart++;
          }
        }

        nx = ii->size[0];
        for (k = 0; k < nx; k++) {
          cluster->b_class->data[ii->data[k] - 1] = c_unique->data[0];
        }

        sortIdx(cluster->b_class, idx);
        ixstart = cluster->b_class->size[0];
        k = c_unique->size[0];
        c_unique->size[0] = ixstart;
        emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
        for (k = 0; k + 1 <= cluster->b_class->size[0]; k++) {
          c_unique->data[k] = cluster->b_class->data[idx->data[k] - 1];
        }

        count_nonfinites(c_unique, cluster->b_class->size[0], &k, &ixstart, &nx,
                         &b_idx);
        nb = -1;
        if (k > 0) {
          nb = 0;
        }

        n = k + ixstart;
        while (k + 1 <= n) {
          b_x = c_unique->data[k];
          do {
            exitg3 = 0;
            k++;
            if (k + 1 > n) {
              exitg3 = 1;
            } else {
              absxk = std::abs(b_x / 2.0);
              if ((!rtIsInf(absxk)) && (!rtIsNaN(absxk))) {
                if (absxk <= 2.2250738585072014E-308) {
                  absxk = 4.94065645841247E-324;
                } else {
                  frexp(absxk, &b_exponent);
                  absxk = std::ldexp(1.0, b_exponent - 53);
                }
              } else {
                absxk = rtNaN;
              }

              if ((std::abs(b_x - c_unique->data[k]) < absxk) ||
                  (rtIsInf(c_unique->data[k]) && rtIsInf(b_x) &&
                   ((c_unique->data[k] > 0.0) == (b_x > 0.0)))) {
                p = true;
              } else {
                p = false;
              }

              if (!p) {
                exitg3 = 1;
              }
            }
          } while (exitg3 == 0);

          nb++;
          c_unique->data[nb] = b_x;
        }

        if (nx > 0) {
          nb++;
          c_unique->data[nb] = c_unique->data[n];
        }

        k = n + nx;
        for (ixstart = 1; ixstart <= b_idx; ixstart++) {
          nb++;
          c_unique->data[nb] = c_unique->data[(k + ixstart) - 1];
        }

        k = c_unique->size[0];
        if (1 > nb + 1) {
          i11 = -1;
        } else {
          i11 = nb;
        }

        c_unique->size[0] = i11 + 1;
        emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
        k = r8->size[0];
        r8->size[0] = c_unique->size[0];
        emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
        nx = c_unique->size[0];
        for (k = 0; k < nx; k++) {
          r8->data[k] = (c_unique->data[k] == 0.0);
        }

        nullAssignment(c_unique, r8);
        ixstart = cluster->b_class->size[0];
        k = b_class->size[0];
        b_class->size[0] = ixstart;
        emxEnsureCapacity((emxArray__common *)b_class, k, (int)sizeof(double));
        for (k = 0; k < ixstart; k++) {
          b_class->data[k] = 0.0;
        }

        ixstart = cluster->area->size[0];
        k = area->size[0];
        area->size[0] = ixstart;
        emxEnsureCapacity((emxArray__common *)area, k, (int)sizeof(double));
        for (k = 0; k < ixstart; k++) {
          area->data[k] = 0.0;
        }

        for (b_idx = 0; b_idx < c_unique->size[0]; b_idx++) {
          absxk = c_unique->data[b_idx];
          k = r8->size[0];
          r8->size[0] = cluster->b_class->size[0];
          emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
          nx = cluster->b_class->size[0];
          for (k = 0; k < nx; k++) {
            r8->data[k] = (cluster->b_class->data[k] == absxk);
          }

          end = r8->size[0];
          for (n = 0; n < end; n++) {
            if (r8->data[n]) {
              b_class->data[n] = 1.0 + (double)b_idx;
            }
          }

          absxk = c_unique->data[b_idx];
          k = r8->size[0];
          r8->size[0] = cluster->b_class->size[0];
          emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
          nx = cluster->b_class->size[0];
          for (k = 0; k < nx; k++) {
            r8->data[k] = (cluster->b_class->data[k] == absxk);
          }

          end = r8->size[0] - 1;
          ixstart = 0;
          for (n = 0; n <= end; n++) {
            if (r8->data[n]) {
              ixstart++;
            }
          }

          k = ii->size[0];
          ii->size[0] = ixstart;
          emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
          ixstart = 0;
          for (n = 0; n <= end; n++) {
            if (r8->data[n]) {
              ii->data[ixstart] = n + 1;
              ixstart++;
            }
          }

          absxk = c_unique->data[b_idx];
          k = c_cluster->size[0];
          c_cluster->size[0] = cluster->b_class->size[0];
          emxEnsureCapacity((emxArray__common *)c_cluster, k,
                            (int)sizeof(boolean_T));
          nx = cluster->b_class->size[0];
          for (k = 0; k < nx; k++) {
            c_cluster->data[k] = (cluster->b_class->data[k] == absxk);
          }

          absxk = sum(c_cluster);
          nx = ii->size[0];
          for (k = 0; k < nx; k++) {
            area->data[ii->data[k] - 1] = absxk;
          }
        }

        k = cluster->b_class->size[0];
        cluster->b_class->size[0] = b_class->size[0];
        emxEnsureCapacity((emxArray__common *)cluster->b_class, k,
                          (int)sizeof(double));
        nx = b_class->size[0];
        for (k = 0; k < nx; k++) {
          cluster->b_class->data[k] = b_class->data[k];
        }

        k = cluster->area->size[0];
        cluster->area->size[0] = area->size[0];
        emxEnsureCapacity((emxArray__common *)cluster->area, k,
                          (int)sizeof(double));
        nx = area->size[0];
        for (k = 0; k < nx; k++) {
          cluster->area->data[k] = area->data[k];
        }
      }
    }
  } while (exitg2 == 0);

  emxFree_boolean_T(&c_cluster);
  emxFree_real_T(&b_MW);
  emxFree_boolean_T(&b_cluster);
  emxFree_real_T(&r9);
  emxFree_boolean_T(&x);
  emxFree_real_T(&C);
  emxFree_real_T(&qeeg);
  ixstart = MW->size[0];
  k = cluster->weight->size[0];
  cluster->weight->size[0] = cluster->area->size[0];
  emxEnsureCapacity((emxArray__common *)cluster->weight, k,
                    (int)sizeof(double));
  nx = cluster->area->size[0];
  for (k = 0; k < nx; k++) {
    cluster->weight->data[k] = 100.0 * cluster->area->data[k] / (double)ixstart;
  }

  sortIdx(cluster->b_class, idx);
  ixstart = cluster->b_class->size[0];
  k = area->size[0];
  area->size[0] = ixstart;
  emxEnsureCapacity((emxArray__common *)area, k, (int)sizeof(double));
  for (k = 0; k + 1 <= cluster->b_class->size[0]; k++) {
    area->data[k] = cluster->b_class->data[idx->data[k] - 1];
  }

  count_nonfinites(area, cluster->b_class->size[0], &k, &ixstart, &nx, &b_idx);
  nb = -1;
  if (k > 0) {
    nb = 0;
  }

  n = k + ixstart;
  while (k + 1 <= n) {
    b_x = area->data[k];
    ixstart = k;
    do {
      exitg1 = 0;
      k++;
      if (k + 1 > n) {
        exitg1 = 1;
      } else {
        absxk = std::abs(b_x / 2.0);
        if ((!rtIsInf(absxk)) && (!rtIsNaN(absxk))) {
          if (absxk <= 2.2250738585072014E-308) {
            absxk = 4.94065645841247E-324;
          } else {
            frexp(absxk, &exponent);
            absxk = std::ldexp(1.0, exponent - 53);
          }
        } else {
          absxk = rtNaN;
        }

        if ((std::abs(b_x - area->data[k]) < absxk) ||
            (rtIsInf(area->data[k]) && rtIsInf(b_x) &&
             ((area->data[k] > 0.0) == (b_x > 0.0)))) {
          p = true;
        } else {
          p = false;
        }

        if (!p) {
          exitg1 = 1;
        }
      }
    } while (exitg1 == 0);

    nb++;
    area->data[nb] = b_x;
    idx->data[nb] = idx->data[ixstart];
  }

  if (nx > 0) {
    nb++;
    area->data[nb] = area->data[n];
    idx->data[nb] = idx->data[n];
  }

  k = n + nx;
  for (ixstart = 0; ixstart + 1 <= b_idx; ixstart++) {
    nb++;
    area->data[nb] = area->data[k + ixstart];
    idx->data[nb] = idx->data[k + ixstart];
  }

  if (1 > nb + 1) {
    end = -1;
  } else {
    end = nb;
  }

  k = area->size[0];
  area->size[0] = end + 1;
  emxEnsureCapacity((emxArray__common *)area, k, (int)sizeof(double));
  k = ii->size[0];
  ii->size[0] = nb + 1;
  emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
  for (k = 0; k + 1 <= nb + 1; k++) {
    ii->data[k] = idx->data[k];
  }

  emxFree_int32_T(&idx);
  k = c_unique->size[0];
  c_unique->size[0] = ii->size[0];
  emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
  nx = ii->size[0];
  for (k = 0; k < nx; k++) {
    c_unique->data[k] = ii->data[k];
  }

  ixstart = 0;
  for (b_idx = 0; b_idx <= end; b_idx++) {
    if (area->data[b_idx] > 0.0) {
      ixstart++;
    }
  }

  k = ii->size[0];
  ii->size[0] = ixstart;
  emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
  ixstart = 0;
  for (b_idx = 0; b_idx <= end; b_idx++) {
    if (area->data[b_idx] > 0.0) {
      ii->data[ixstart] = b_idx + 1;
      ixstart++;
    }
  }

  k = b_class->size[0];
  b_class->size[0] = ii->size[0];
  emxEnsureCapacity((emxArray__common *)b_class, k, (int)sizeof(double));
  nx = ii->size[0];
  for (k = 0; k < nx; k++) {
    b_class->data[k] =
        cluster->weight->data[(int)c_unique->data[ii->data[k] - 1] - 1];
  }

  emxInit_boolean_T(&b_area, 1);
  k = b_area->size[0];
  b_area->size[0] = area->size[0];
  emxEnsureCapacity((emxArray__common *)b_area, k, (int)sizeof(boolean_T));
  nx = area->size[0];
  for (k = 0; k < nx; k++) {
    b_area->data[k] = (area->data[k] == 0.0);
  }

  nullAssignment(area, b_area);
  f_sort(b_class, ii);
  k = c_unique->size[0];
  c_unique->size[0] = ii->size[0];
  emxEnsureCapacity((emxArray__common *)c_unique, k, (int)sizeof(double));
  nx = ii->size[0];
  emxFree_boolean_T(&b_area);
  for (k = 0; k < nx; k++) {
    c_unique->data[k] = ii->data[k];
  }

  k = b_class->size[0];
  b_class->size[0] = cluster->b_class->size[0];
  emxEnsureCapacity((emxArray__common *)b_class, k, (int)sizeof(double));
  nx = cluster->b_class->size[0];
  for (k = 0; k < nx; k++) {
    b_class->data[k] = cluster->b_class->data[k];
  }

  for (b_idx = 0; b_idx < c_unique->size[0]; b_idx++) {
    absxk = area->data[(int)c_unique->data[b_idx] - 1];
    k = r8->size[0];
    r8->size[0] = b_class->size[0];
    emxEnsureCapacity((emxArray__common *)r8, k, (int)sizeof(boolean_T));
    nx = b_class->size[0];
    for (k = 0; k < nx; k++) {
      r8->data[k] = (b_class->data[k] == absxk);
    }

    end = r8->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= end; n++) {
      if (r8->data[n]) {
        ixstart++;
      }
    }

    k = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, k, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= end; n++) {
      if (r8->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    nx = ii->size[0];
    for (k = 0; k < nx; k++) {
      cluster->b_class->data[ii->data[k] - 1] = (unsigned int)(1 + b_idx);
    }
  }

  emxFree_int32_T(&ii);
  emxFree_boolean_T(&r8);
  emxFree_real_T(&area);
  emxFree_real_T(&b_class);
  emxFree_real_T(&c_unique);
}

//
// MY_CORR This is a trivial implementation of corr() via corrcoef().
// Arguments    : const emxArray_real_T *x
//                const emxArray_real_T *y
//                emxArray_real_T *res
// Return Type  : void
//
static void my_corr(const emxArray_real_T *x, const emxArray_real_T *y,
                    emxArray_real_T *res) {
  int i4;
  int i;
  emxArray_real_T *result;
  emxArray_real_T *b_x;
  emxArray_real_T *c_x;
  emxArray_real_T *b_y;
  int j;
  int loop_ub;
  int b_loop_ub;
  int fm;
  int n;
  double xy[4];
  double d;
  double b_d[2];
  i4 = res->size[0] * res->size[1];
  res->size[0] = x->size[1];
  res->size[1] = y->size[1];
  emxEnsureCapacity((emxArray__common *)res, i4, (int)sizeof(double));
  i = 0;
  emxInit_real_T(&result, 2);
  emxInit_real_T(&b_x, 2);
  emxInit_real_T1(&c_x, 1);
  emxInit_real_T1(&b_y, 1);
  while (i <= x->size[1] - 1) {
    for (j = 0; j < y->size[1]; j++) {
      loop_ub = x->size[0];
      b_loop_ub = y->size[0];
      fm = x->size[0];
      i4 = c_x->size[0];
      c_x->size[0] = fm;
      emxEnsureCapacity((emxArray__common *)c_x, i4, (int)sizeof(double));
      for (i4 = 0; i4 < fm; i4++) {
        c_x->data[i4] = x->data[i4 + x->size[0] * i];
      }

      fm = y->size[0];
      i4 = b_y->size[0];
      b_y->size[0] = fm;
      emxEnsureCapacity((emxArray__common *)b_y, i4, (int)sizeof(double));
      for (i4 = 0; i4 < fm; i4++) {
        b_y->data[i4] = y->data[i4 + y->size[0] * j];
      }

      i4 = result->size[0] * result->size[1];
      result->size[0] = loop_ub;
      result->size[1] = 2;
      emxEnsureCapacity((emxArray__common *)result, i4, (int)sizeof(double));
      for (i4 = 0; i4 < loop_ub; i4++) {
        result->data[i4] = c_x->data[i4];
      }

      for (i4 = 0; i4 < b_loop_ub; i4++) {
        result->data[i4 + result->size[0]] = b_y->data[i4];
      }

      i4 = b_x->size[0] * b_x->size[1];
      b_x->size[0] = result->size[0];
      b_x->size[1] = 2;
      emxEnsureCapacity((emxArray__common *)b_x, i4, (int)sizeof(double));
      loop_ub = result->size[0] * result->size[1];
      for (i4 = 0; i4 < loop_ub; i4++) {
        b_x->data[i4] = result->data[i4];
      }

      n = result->size[0];
      for (i4 = 0; i4 < 4; i4++) {
        xy[i4] = 0.0;
      }

      if (result->size[0] < 2) {
        for (i4 = 0; i4 < 4; i4++) {
          xy[i4] = rtNaN;
        }
      } else {
        for (b_loop_ub = 0; b_loop_ub < 2; b_loop_ub++) {
          d = 0.0;
          for (loop_ub = 1; loop_ub <= n; loop_ub++) {
            d += b_x->data[(loop_ub + b_x->size[0] * b_loop_ub) - 1];
          }

          d /= (double)result->size[0];
          for (loop_ub = 0; loop_ub + 1 <= n; loop_ub++) {
            b_x->data[loop_ub + b_x->size[0] * b_loop_ub] -= d;
          }
        }

        fm = result->size[0] - 1;
        for (b_loop_ub = 0; b_loop_ub < 2; b_loop_ub++) {
          d = 0.0;
          for (loop_ub = 0; loop_ub + 1 <= n; loop_ub++) {
            d += b_x->data[loop_ub + b_x->size[0] * b_loop_ub] *
                 b_x->data[loop_ub + b_x->size[0] * b_loop_ub];
          }

          xy[b_loop_ub + (b_loop_ub << 1)] = d / (double)fm;
          loop_ub = b_loop_ub + 2;
          while (loop_ub < 3) {
            d = 0.0;
            for (loop_ub = 0; loop_ub + 1 <= n; loop_ub++) {
              d += b_x->data[loop_ub + b_x->size[0]] *
                   b_x->data[loop_ub + b_x->size[0] * b_loop_ub];
            }

            xy[1 + (b_loop_ub << 1)] = d / (double)fm;
            loop_ub = 3;
          }
        }
      }

      for (loop_ub = 0; loop_ub < 2; loop_ub++) {
        b_d[loop_ub] = std::sqrt(xy[loop_ub + (loop_ub << 1)]);
      }

      for (b_loop_ub = 0; b_loop_ub < 2; b_loop_ub++) {
        loop_ub = b_loop_ub + 2;
        while (loop_ub < 3) {
          xy[1 + (b_loop_ub << 1)] =
              xy[1 + (b_loop_ub << 1)] / b_d[1] / b_d[b_loop_ub];
          loop_ub = 3;
        }

        loop_ub = b_loop_ub + 2;
        while (loop_ub < 3) {
          d = std::abs(xy[1 + (b_loop_ub << 1)]);
          if (d > 1.0) {
            xy[1 + (b_loop_ub << 1)] /= d;
          }

          xy[2 + b_loop_ub] = xy[1 + (b_loop_ub << 1)];
          loop_ub = 3;
        }

        if (xy[b_loop_ub + (b_loop_ub << 1)] > 0.0) {
          if (xy[b_loop_ub + (b_loop_ub << 1)] < 0.0) {
            xy[b_loop_ub + (b_loop_ub << 1)] = -1.0;
          } else if (xy[b_loop_ub + (b_loop_ub << 1)] > 0.0) {
            xy[b_loop_ub + (b_loop_ub << 1)] = 1.0;
          } else {
            if (xy[b_loop_ub + (b_loop_ub << 1)] == 0.0) {
              xy[b_loop_ub + (b_loop_ub << 1)] = 0.0;
            }
          }
        } else {
          xy[b_loop_ub + (b_loop_ub << 1)] = rtNaN;
        }
      }

      res->data[i + res->size[0] * j] = xy[2];
    }

    i++;
  }

  emxFree_real_T(&b_y);
  emxFree_real_T(&c_x);
  emxFree_real_T(&b_x);
  emxFree_real_T(&result);
}

//
// REPEAT_IF_SMALLER Repeat the first element n times if x is not of length n.
// Arguments    : emxArray_real_T *x
//                double n
// Return Type  : void
//
static void repeat_if_smaller(emxArray_real_T *x, double n) {
  double b_x;
  int i10;
  int loop_ub;
  if (x->size[0] != n) {
    cfprintf();
    b_x = x->data[0];
    i10 = x->size[0];
    x->size[0] = (int)n;
    emxEnsureCapacity((emxArray__common *)x, i10, (int)sizeof(double));
    loop_ub = (int)n;
    for (i10 = 0; i10 < loop_ub; i10++) {
      x->data[i10] = b_x;
    }
  }
}

//
// Arguments    : emxArray_real_T *data
//                double pca_centering
//                emxArray_real_T *b_class
//                emxArray_real_T *w
//                emxArray_real_T *area
// Return Type  : void
//
static void spike_cluster_sub(emxArray_real_T *data, double pca_centering,
                              emxArray_real_T *b_class, emxArray_real_T *w,
                              emxArray_real_T *area) {
  emxArray_real_T *sub_data;
  emxArray_real_T *rand_eigval;
  emxArray_real_T *unusedU6;
  emxArray_real_T *unusedU5;
  emxArray_real_T *b_sub_data;
  emxArray_real_T *b_rand_eigval;
  emxArray_real_T *x;
  emxArray_real_T *y;
  emxArray_real_T *b_data;
  emxArray_boolean_T *r3;
  int itmp;
  int loop_ub;
  emxArray_boolean_T *ch_err;
  emxArray_real_T *b;
  emxArray_int32_T *r4;
  int end;
  int vstride;
  int ixstop;
  int n;
  emxArray_real_T *c_data;
  emxArray_real_T *b_pca_centering;
  emxArray_real_T *C;
  emxArray_real_T *d_data;
  emxArray_real_T *D_pca;
  emxArray_real_T *eigval;
  double re[100];
  int i;
  emxArray_boolean_T *b_eigval;
  double b_y;
  double d0;
  emxArray_real_T *e_data;
  double n_cluster;
  emxArray_int32_T *iindx;
  int sz[2];
  int b_n;
  int ixstart;
  unsigned int unnamed_idx_0;
  int i1;
  int b_loop_ub;
  emxArray_int32_T *indx;
  int cindx;
  int ix;
  double c_y;
  boolean_T exitg2;
  emxArray_real_T *extremum;
  boolean_T exitg3;
  emxArray_int32_T *c;
  boolean_T exitg1;
  emxArray_int32_T *r5;
  emxArray_real_T *c_eigval;
  emxArray_real_T *b_extremum;
  unsigned int b_unnamed_idx_0;
  emxArray_boolean_T *b_c;
  emxArray_int32_T *iidx;
  emxArray_boolean_T *c_class;
  emxArray_boolean_T *d_class;
  spike_cluster_cppTLS *spike_cluster_cppTLSThread;
  spike_cluster_cppTLSThread = emlrtGetThreadStackData();
  emxInit_real_T(&sub_data, 2);
  emxInit_real_T1(&rand_eigval, 1);
  emxInit_real_T(&unusedU6, 2);
  emxInit_real_T(&unusedU5, 2);
  emxInit_real_T(&b_sub_data, 2);
  emxInit_real_T1(&b_rand_eigval, 1);
  emxInit_real_T(&x, 2);
  emxInit_real_T(&y, 2);
  emxInit_real_T(&b_data, 2);
  if (data->size[0] == 1) {
    //  only one realization
    itmp = b_class->size[0];
    b_class->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)b_class, itmp, (int)sizeof(double));
    b_class->data[0] = 1.0;
    itmp = w->size[0];
    w->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)w, itmp, (int)sizeof(double));
    w->data[0] = 100.0;
    itmp = area->size[0];
    area->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)area, itmp, (int)sizeof(double));
    area->data[0] = 1.0;
  } else {
    emxInit_boolean_T1(&r3, 2);
    itmp = r3->size[0] * r3->size[1];
    r3->size[0] = data->size[0];
    r3->size[1] = data->size[1];
    emxEnsureCapacity((emxArray__common *)r3, itmp, (int)sizeof(boolean_T));
    loop_ub = data->size[0] * data->size[1];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      r3->data[itmp] = rtIsNaN(data->data[itmp]);
    }

    emxInit_boolean_T1(&ch_err, 2);
    emxInit_real_T(&b, 2);
    c_sum(r3, b);
    itmp = ch_err->size[0] * ch_err->size[1];
    ch_err->size[0] = 1;
    ch_err->size[1] = b->size[1];
    emxEnsureCapacity((emxArray__common *)ch_err, itmp, (int)sizeof(boolean_T));
    loop_ub = b->size[0] * b->size[1];
    emxFree_boolean_T(&r3);
    for (itmp = 0; itmp < loop_ub; itmp++) {
      ch_err->data[itmp] = (b->data[itmp] == 0.0);
    }

    emxInit_int32_T1(&r4, 2);

    //  removing NaN values
    end = ch_err->size[1] - 1;
    vstride = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (ch_err->data[ixstop]) {
        vstride++;
      }
    }

    itmp = r4->size[0] * r4->size[1];
    r4->size[0] = 1;
    r4->size[1] = vstride;
    emxEnsureCapacity((emxArray__common *)r4, itmp, (int)sizeof(int));
    n = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (ch_err->data[ixstop]) {
        r4->data[n] = ixstop + 1;
        n++;
      }
    }

    emxFree_boolean_T(&ch_err);
    emxInit_real_T(&c_data, 2);
    vstride = data->size[0];
    itmp = c_data->size[0] * c_data->size[1];
    c_data->size[0] = vstride;
    c_data->size[1] = r4->size[1];
    emxEnsureCapacity((emxArray__common *)c_data, itmp, (int)sizeof(double));
    loop_ub = r4->size[1];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      for (n = 0; n < vstride; n++) {
        c_data->data[n + c_data->size[0] * itmp] =
            data->data[n + data->size[0] * (r4->data[r4->size[0] * itmp] - 1)];
      }
    }

    emxFree_int32_T(&r4);
    itmp = data->size[0] * data->size[1];
    data->size[0] = c_data->size[0];
    data->size[1] = c_data->size[1];
    emxEnsureCapacity((emxArray__common *)data, itmp, (int)sizeof(double));
    loop_ub = c_data->size[1];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      vstride = c_data->size[0];
      for (n = 0; n < vstride; n++) {
        data->data[n + data->size[0] * itmp] =
            c_data->data[n + c_data->size[0] * itmp];
      }
    }

    emxFree_real_T(&c_data);
    emxInit_real_T(&b_pca_centering, 2);
    mean(data, b);
    itmp = b_pca_centering->size[0] * b_pca_centering->size[1];
    b_pca_centering->size[0] = 1;
    b_pca_centering->size[1] = b->size[1];
    emxEnsureCapacity((emxArray__common *)b_pca_centering, itmp,
                      (int)sizeof(double));
    loop_ub = b->size[0] * b->size[1];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      b_pca_centering->data[itmp] = pca_centering * b->data[itmp];
    }

    emxFree_real_T(&b);
    emxInit_real_T(&C, 2);
    repmat(b_pca_centering, (double)data->size[0], C);
    itmp = data->size[0] * data->size[1];
    emxEnsureCapacity((emxArray__common *)data, itmp, (int)sizeof(double));
    vstride = data->size[0];
    n = data->size[1];
    loop_ub = vstride * n;
    emxFree_real_T(&b_pca_centering);
    for (itmp = 0; itmp < loop_ub; itmp++) {
      data->data[itmp] -= C->data[itmp];
    }

    emxInit_real_T(&d_data, 2);

    //  CENTERING
    itmp = d_data->size[0] * d_data->size[1];
    d_data->size[0] = data->size[1];
    d_data->size[1] = data->size[0];
    emxEnsureCapacity((emxArray__common *)d_data, itmp, (int)sizeof(double));
    loop_ub = data->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      vstride = data->size[1];
      for (n = 0; n < vstride; n++) {
        d_data->data[n + d_data->size[0] * itmp] =
            data->data[itmp + data->size[0] * n];
      }
    }

    emxInit_real_T(&D_pca, 2);
    emxInit_real_T1(&eigval, 1);
    pca(d_data, C, D_pca, eigval);
    emxFree_real_T(&d_data);
    if (!omp_in_parallel()) {
      c_eml_rand_mt19937ar_stateful_s(true);
      eml_rand_swap();
    }

#pragma omp parallel num_threads(omp_get_max_threads()) private(               \
    spike_cluster_cppTLSThread, b_data, b_rand_eigval, y, x, unusedU6,         \
    sub_data, b_sub_data, unusedU5, rand_eigval, b_n, ixstart, i1, b_loop_ub,  \
    c_y, exitg3) firstprivate(sz)

    {
      spike_cluster_cppTLSThread = emlrtGetThreadStackData();
      emxInit_real_T(&b_data, 2);
      emxInit_real_T1(&b_rand_eigval, 1);
      emxInit_real_T(&y, 2);
      emxInit_real_T(&x, 2);
      emxInit_real_T(&unusedU6, 2);
      emxInit_real_T(&sub_data, 2);
      emxInit_real_T(&b_sub_data, 2);
      emxInit_real_T(&unusedU5, 2);
      emxInit_real_T1(&rand_eigval, 1);

#pragma omp for nowait

      for (i = 0; i < 100; i++) {
        //    disp(['PCA threshold: ' num2str(i) '/100'])
        if (data->size[0] > 50000) {
          b_randperm((double)data->size[0], spike_cluster_cppTLSThread->f1.dv1);
          b_n = data->size[1];
          ixstart = sub_data->size[0] * sub_data->size[1];
          sub_data->size[0] = 50000;
          sub_data->size[1] = b_n;
          emxEnsureCapacity((emxArray__common *)sub_data, ixstart,
                            (int)sizeof(double));
          for (ixstart = 0; ixstart < b_n; ixstart++) {
            for (i1 = 0; i1 < 50000; i1++) {
              sub_data->data[i1 + sub_data->size[0] * ixstart] =
                  data->data[((int)spike_cluster_cppTLSThread->f1.dv1[i1] +
                              data->size[0] * ixstart) -
                             1];
            }
          }

          c_randperm((double)(50000 * sub_data->size[1]), x);
          for (ixstart = 0; ixstart < 2; ixstart++) {
            sz[ixstart] = sub_data->size[ixstart];
          }

          ixstart = y->size[0] * y->size[1];
          y->size[0] = 50000;
          y->size[1] = sz[1];
          emxEnsureCapacity((emxArray__common *)y, ixstart,
                            (int)sizeof(double));
          for (ixstart = 0; ixstart + 1 <= x->size[1]; ixstart++) {
            y->data[ixstart] = x->data[ixstart];
          }

          ixstart = b_sub_data->size[0] * b_sub_data->size[1];
          b_sub_data->size[0] = y->size[1];
          b_sub_data->size[1] = y->size[0];
          emxEnsureCapacity((emxArray__common *)b_sub_data, ixstart,
                            (int)sizeof(double));
          b_n = y->size[0];
          for (ixstart = 0; ixstart < b_n; ixstart++) {
            b_loop_ub = y->size[1];
            for (i1 = 0; i1 < b_loop_ub; i1++) {
              b_sub_data->data[i1 + b_sub_data->size[0] * ixstart] =
                  sub_data->data[(int)y->data[ixstart + y->size[0] * i1] - 1];
            }
          }

          pca(b_sub_data, unusedU6, unusedU5, b_rand_eigval);
          ixstart = rand_eigval->size[0];
          rand_eigval->size[0] = b_rand_eigval->size[0];
          emxEnsureCapacity((emxArray__common *)rand_eigval, ixstart,
                            (int)sizeof(double));
          b_n = b_rand_eigval->size[0];
          for (ixstart = 0; ixstart < b_n; ixstart++) {
            rand_eigval->data[ixstart] = b_rand_eigval->data[ixstart];
          }
        } else {
          c_randperm((double)(data->size[0] * data->size[1]), x);
          for (ixstart = 0; ixstart < 2; ixstart++) {
            sz[ixstart] = data->size[ixstart];
          }

          ixstart = y->size[0] * y->size[1];
          y->size[0] = sz[0];
          y->size[1] = sz[1];
          emxEnsureCapacity((emxArray__common *)y, ixstart,
                            (int)sizeof(double));
          for (ixstart = 0; ixstart + 1 <= x->size[1]; ixstart++) {
            y->data[ixstart] = x->data[ixstart];
          }

          ixstart = b_data->size[0] * b_data->size[1];
          b_data->size[0] = y->size[1];
          b_data->size[1] = y->size[0];
          emxEnsureCapacity((emxArray__common *)b_data, ixstart,
                            (int)sizeof(double));
          b_n = y->size[0];
          for (ixstart = 0; ixstart < b_n; ixstart++) {
            b_loop_ub = y->size[1];
            for (i1 = 0; i1 < b_loop_ub; i1++) {
              b_data->data[i1 + b_data->size[0] * ixstart] =
                  data->data[(int)y->data[ixstart + y->size[0] * i1] - 1];
            }
          }

          pca(b_data, unusedU6, unusedU5, b_rand_eigval);
          ixstart = rand_eigval->size[0];
          rand_eigval->size[0] = b_rand_eigval->size[0];
          emxEnsureCapacity((emxArray__common *)rand_eigval, ixstart,
                            (int)sizeof(double));
          b_n = b_rand_eigval->size[0];
          for (ixstart = 0; ixstart < b_n; ixstart++) {
            rand_eigval->data[ixstart] = b_rand_eigval->data[ixstart];
          }
        }

        c_y = d_sum(rand_eigval);
        ixstart = b_rand_eigval->size[0];
        b_rand_eigval->size[0] = rand_eigval->size[0];
        emxEnsureCapacity((emxArray__common *)b_rand_eigval, ixstart,
                          (int)sizeof(double));
        b_n = rand_eigval->size[0];
        for (ixstart = 0; ixstart < b_n; ixstart++) {
          b_rand_eigval->data[ixstart] = rand_eigval->data[ixstart] / c_y;
        }

        ixstart = 1;
        b_n = b_rand_eigval->size[0];
        c_y = b_rand_eigval->data[0];
        if (b_rand_eigval->size[0] > 1) {
          if (rtIsNaN(b_rand_eigval->data[0])) {
            b_loop_ub = 2;
            exitg3 = false;
            while ((!exitg3) && (b_loop_ub <= b_n)) {
              ixstart = b_loop_ub;
              if (!rtIsNaN(b_rand_eigval->data[b_loop_ub - 1])) {
                c_y = b_rand_eigval->data[b_loop_ub - 1];
                exitg3 = true;
              } else {
                b_loop_ub++;
              }
            }
          }

          if (ixstart < b_rand_eigval->size[0]) {
            while (ixstart + 1 <= b_n) {
              if (b_rand_eigval->data[ixstart] > c_y) {
                c_y = b_rand_eigval->data[ixstart];
              }

              ixstart++;
            }
          }
        }

        re[i] = c_y;
      }

      emxFree_real_T(&rand_eigval);
      emxFree_real_T(&unusedU5);
      emxFree_real_T(&b_sub_data);
      emxFree_real_T(&sub_data);
      emxFree_real_T(&unusedU6);
      emxFree_real_T(&x);
      emxFree_real_T(&y);
      emxFree_real_T(&b_rand_eigval);
      emxFree_real_T(&b_data);
    }

    if (!omp_in_parallel()) {
      c_eml_rand_mt19937ar_stateful_s(false);
      eml_rand_swap();
    }

    emxInit_boolean_T(&b_eigval, 1);

    //  eigval=eigval/sum(eigval); % eigenvalue normalization to 100%
    //  n_cluster=sum(eigval>mean(re)); % significant eigenvalue treshold
    //  n_cluster=sum((eigval/sum(eigval))>mean(re)); % significant eigenvalue
    //  treshold
    b_y = d_sum(eigval);
    d0 = quantile(re);
    itmp = b_eigval->size[0];
    b_eigval->size[0] = eigval->size[0];
    emxEnsureCapacity((emxArray__common *)b_eigval, itmp,
                      (int)sizeof(boolean_T));
    loop_ub = eigval->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      b_eigval->data[itmp] = (eigval->data[itmp] / b_y > d0);
    }

    emxInit_real_T(&e_data, 2);
    n_cluster = sum(b_eigval);

    //  significant eigenvalue treshold
    // C=corr(data',D_pca); % eigenvectors correlation to data
    itmp = e_data->size[0] * e_data->size[1];
    e_data->size[0] = data->size[1];
    e_data->size[1] = data->size[0];
    emxEnsureCapacity((emxArray__common *)e_data, itmp, (int)sizeof(double));
    loop_ub = data->size[0];
    emxFree_boolean_T(&b_eigval);
    for (itmp = 0; itmp < loop_ub; itmp++) {
      vstride = data->size[1];
      for (n = 0; n < vstride; n++) {
        e_data->data[n + e_data->size[0] * itmp] =
            data->data[itmp + data->size[0] * n];
      }
    }

    emxInit_int32_T(&iindx, 1);
    my_corr(e_data, D_pca, C);

    //  positive
    itmp = eigval->size[0];
    eigval->size[0] = C->size[0];
    emxEnsureCapacity((emxArray__common *)eigval, itmp, (int)sizeof(double));
    unnamed_idx_0 = (unsigned int)eigval->size[0];
    itmp = iindx->size[0];
    iindx->size[0] = (int)unnamed_idx_0;
    emxEnsureCapacity((emxArray__common *)iindx, itmp, (int)sizeof(int));
    loop_ub = (int)unnamed_idx_0;
    emxFree_real_T(&e_data);
    emxFree_real_T(&D_pca);
    for (itmp = 0; itmp < loop_ub; itmp++) {
      iindx->data[itmp] = 1;
    }

    n = C->size[1];
    vstride = C->size[0];
    for (loop_ub = 0; loop_ub + 1 <= vstride; loop_ub++) {
      end = loop_ub;
      ixstop = (loop_ub + (n - 1) * vstride) + 1;
      b_y = C->data[loop_ub];
      itmp = 1;
      if (n > 1) {
        cindx = 1;
        if (rtIsNaN(C->data[loop_ub])) {
          ix = loop_ub + vstride;
          exitg2 = false;
          while ((!exitg2) && ((vstride > 0) && (ix + 1 <= ixstop))) {
            cindx++;
            end = ix;
            if (!rtIsNaN(C->data[ix])) {
              b_y = C->data[ix];
              itmp = cindx;
              exitg2 = true;
            } else {
              ix += vstride;
            }
          }
        }

        if (end + 1 < ixstop) {
          ix = end + vstride;
          while ((vstride > 0) && (ix + 1 <= ixstop)) {
            cindx++;
            if (C->data[ix] > b_y) {
              b_y = C->data[ix];
              itmp = cindx;
            }

            ix += vstride;
          }
        }
      }

      eigval->data[loop_ub] = b_y;
      iindx->data[loop_ub] = itmp;
    }

    emxInit_int32_T(&indx, 1);
    itmp = indx->size[0];
    indx->size[0] = iindx->size[0];
    emxEnsureCapacity((emxArray__common *)indx, itmp, (int)sizeof(int));
    loop_ub = iindx->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      indx->data[itmp] = iindx->data[itmp];
    }

    //  maximal correlation of each positive eigenvectors
    //  negative
    itmp = C->size[0] * C->size[1];
    emxEnsureCapacity((emxArray__common *)C, itmp, (int)sizeof(double));
    n = C->size[0];
    vstride = C->size[1];
    loop_ub = n * vstride;
    for (itmp = 0; itmp < loop_ub; itmp++) {
      C->data[itmp] = -C->data[itmp];
    }

    emxInit_real_T1(&extremum, 1);
    itmp = extremum->size[0];
    extremum->size[0] = C->size[0];
    emxEnsureCapacity((emxArray__common *)extremum, itmp, (int)sizeof(double));
    unnamed_idx_0 = (unsigned int)extremum->size[0];
    itmp = iindx->size[0];
    iindx->size[0] = (int)unnamed_idx_0;
    emxEnsureCapacity((emxArray__common *)iindx, itmp, (int)sizeof(int));
    loop_ub = (int)unnamed_idx_0;
    for (itmp = 0; itmp < loop_ub; itmp++) {
      iindx->data[itmp] = 1;
    }

    n = C->size[1];
    vstride = C->size[0];
    for (loop_ub = 0; loop_ub + 1 <= vstride; loop_ub++) {
      end = loop_ub;
      ixstop = (loop_ub + (n - 1) * vstride) + 1;
      b_y = C->data[loop_ub];
      itmp = 1;
      if (n > 1) {
        cindx = 1;
        if (rtIsNaN(C->data[loop_ub])) {
          ix = loop_ub + vstride;
          exitg1 = false;
          while ((!exitg1) && ((vstride > 0) && (ix + 1 <= ixstop))) {
            cindx++;
            end = ix;
            if (!rtIsNaN(C->data[ix])) {
              b_y = C->data[ix];
              itmp = cindx;
              exitg1 = true;
            } else {
              ix += vstride;
            }
          }
        }

        if (end + 1 < ixstop) {
          ix = end + vstride;
          while ((vstride > 0) && (ix + 1 <= ixstop)) {
            cindx++;
            if (C->data[ix] > b_y) {
              b_y = C->data[ix];
              itmp = cindx;
            }

            ix += vstride;
          }
        }
      }

      extremum->data[loop_ub] = b_y;
      iindx->data[loop_ub] = itmp;
    }

    emxInit_int32_T(&c, 1);
    itmp = c->size[0];
    c->size[0] = iindx->size[0];
    emxEnsureCapacity((emxArray__common *)c, itmp, (int)sizeof(int));
    loop_ub = iindx->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      c->data[itmp] = iindx->data[itmp];
    }

    //  maximal correlation of each neagative eigenvectors
    end = indx->size[0] - 1;
    vstride = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (indx->data[ixstop] <= n_cluster) {
        vstride++;
      }
    }

    itmp = iindx->size[0];
    iindx->size[0] = vstride;
    emxEnsureCapacity((emxArray__common *)iindx, itmp, (int)sizeof(int));
    n = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (indx->data[ixstop] <= n_cluster) {
        iindx->data[n] = ixstop + 1;
        n++;
      }
    }

    emxInit_int32_T(&r5, 1);
    end = c->size[0] - 1;
    vstride = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (c->data[ixstop] < n_cluster) {
        vstride++;
      }
    }

    itmp = r5->size[0];
    r5->size[0] = vstride;
    emxEnsureCapacity((emxArray__common *)r5, itmp, (int)sizeof(int));
    n = 0;
    for (ixstop = 0; ixstop <= end; ixstop++) {
      if (c->data[ixstop] < n_cluster) {
        r5->data[n] = ixstop + 1;
        n++;
      }
    }

    emxInit_real_T1(&c_eigval, 1);
    itmp = c_eigval->size[0];
    c_eigval->size[0] = iindx->size[0];
    emxEnsureCapacity((emxArray__common *)c_eigval, itmp, (int)sizeof(double));
    loop_ub = iindx->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      c_eigval->data[itmp] = eigval->data[iindx->data[itmp] - 1];
    }

    emxFree_real_T(&eigval);
    emxInit_real_T1(&b_extremum, 1);
    itmp = b_extremum->size[0];
    b_extremum->size[0] = r5->size[0];
    emxEnsureCapacity((emxArray__common *)b_extremum, itmp,
                      (int)sizeof(double));
    loop_ub = r5->size[0];
    for (itmp = 0; itmp < loop_ub; itmp++) {
      b_extremum->data[itmp] = extremum->data[r5->data[itmp] - 1];
    }

    emxFree_real_T(&extremum);
    emxFree_int32_T(&r5);
    if (d_sum(c_eigval) > d_sum(b_extremum)) {
      //  use sign of better correlated eigenvectors
      itmp = c->size[0];
      c->size[0] = indx->size[0];
      emxEnsureCapacity((emxArray__common *)c, itmp, (int)sizeof(int));
      loop_ub = indx->size[0];
      for (itmp = 0; itmp < loop_ub; itmp++) {
        c->data[itmp] = indx->data[itmp];
      }
    }

    emxFree_real_T(&b_extremum);
    emxFree_real_T(&c_eigval);
    emxFree_int32_T(&indx);

    //  ------------------
    //  cluster info
    end = c->size[0];
    for (ixstop = 0; ixstop < end; ixstop++) {
      if (c->data[ixstop] > n_cluster) {
        c->data[ixstop] = 0;
      }
    }

    unnamed_idx_0 = (unsigned int)c->size[0];
    itmp = b_class->size[0];
    b_class->size[0] = (int)unnamed_idx_0;
    emxEnsureCapacity((emxArray__common *)b_class, itmp, (int)sizeof(double));
    loop_ub = (int)unnamed_idx_0;
    for (itmp = 0; itmp < loop_ub; itmp++) {
      b_class->data[itmp] = 0.0;
    }

    b_unnamed_idx_0 = (unsigned int)c->size[0];
    itmp = area->size[0];
    area->size[0] = (int)b_unnamed_idx_0;
    emxEnsureCapacity((emxArray__common *)area, itmp, (int)sizeof(double));
    loop_ub = (int)b_unnamed_idx_0;
    for (itmp = 0; itmp < loop_ub; itmp++) {
      area->data[itmp] = 0.0;
    }

    if (n_cluster == 0.0) {
      itmp = area->size[0];
      area->size[0] = 1;
      emxEnsureCapacity((emxArray__common *)area, itmp, (int)sizeof(double));
      area->data[0] = data->size[0];
      itmp = w->size[0];
      w->size[0] = (int)unnamed_idx_0;
      emxEnsureCapacity((emxArray__common *)w, itmp, (int)sizeof(double));
      loop_ub = (int)unnamed_idx_0;
      for (itmp = 0; itmp < loop_ub; itmp++) {
        w->data[itmp] = 100.0;
      }
    } else {
      itmp = C->size[0] * C->size[1];
      C->size[0] = (int)n_cluster;
      C->size[1] = (int)n_cluster;
      emxEnsureCapacity((emxArray__common *)C, itmp, (int)sizeof(double));
      loop_ub = (int)n_cluster * (int)n_cluster;
      for (itmp = 0; itmp < loop_ub; itmp++) {
        C->data[itmp] = 0.0;
      }

      ixstop = 0;
      emxInit_boolean_T(&b_c, 1);
      while (ixstop <= (int)n_cluster - 1) {
        itmp = b_c->size[0];
        b_c->size[0] = c->size[0];
        emxEnsureCapacity((emxArray__common *)b_c, itmp,
                          (int)sizeof(boolean_T));
        loop_ub = c->size[0];
        for (itmp = 0; itmp < loop_ub; itmp++) {
          b_c->data[itmp] = (c->data[itmp] == 1.0 + (double)ixstop);
        }

        C->data[ixstop] = sum(b_c);
        ixstop++;
      }

      emxFree_boolean_T(&b_c);
      emxInit_int32_T1(&iidx, 2);
      d_sort(C, iidx);
      itmp = C->size[0] * C->size[1];
      C->size[0] = iidx->size[0];
      C->size[1] = iidx->size[1];
      emxEnsureCapacity((emxArray__common *)C, itmp, (int)sizeof(double));
      loop_ub = iidx->size[0] * iidx->size[1];
      for (itmp = 0; itmp < loop_ub; itmp++) {
        C->data[itmp] = iidx->data[itmp];
      }

      emxFree_int32_T(&iidx);
      ixstop = 0;
      emxInit_boolean_T(&c_class, 1);
      while (ixstop <= (int)n_cluster - 1) {
        end = c->size[0];
        for (loop_ub = 0; loop_ub < end; loop_ub++) {
          if (c->data[loop_ub] == (int)C->data[ixstop]) {
            b_class->data[loop_ub] = 1.0 + (double)ixstop;
          }
        }

        end = b_class->size[0] - 1;
        vstride = 0;
        for (loop_ub = 0; loop_ub <= end; loop_ub++) {
          if (b_class->data[loop_ub] == 1.0 + (double)ixstop) {
            vstride++;
          }
        }

        itmp = iindx->size[0];
        iindx->size[0] = vstride;
        emxEnsureCapacity((emxArray__common *)iindx, itmp, (int)sizeof(int));
        n = 0;
        for (loop_ub = 0; loop_ub <= end; loop_ub++) {
          if (b_class->data[loop_ub] == 1.0 + (double)ixstop) {
            iindx->data[n] = loop_ub + 1;
            n++;
          }
        }

        itmp = c_class->size[0];
        c_class->size[0] = b_class->size[0];
        emxEnsureCapacity((emxArray__common *)c_class, itmp,
                          (int)sizeof(boolean_T));
        loop_ub = b_class->size[0];
        for (itmp = 0; itmp < loop_ub; itmp++) {
          c_class->data[itmp] = (b_class->data[itmp] == 1.0 + (double)ixstop);
        }

        d0 = sum(c_class);
        loop_ub = iindx->size[0];
        for (itmp = 0; itmp < loop_ub; itmp++) {
          area->data[iindx->data[itmp] - 1] = d0;
        }

        ixstop++;
      }

      emxFree_boolean_T(&c_class);
      end = b_class->size[0] - 1;
      vstride = 0;
      for (ixstop = 0; ixstop <= end; ixstop++) {
        if (b_class->data[ixstop] == 0.0) {
          vstride++;
        }
      }

      itmp = iindx->size[0];
      iindx->size[0] = vstride;
      emxEnsureCapacity((emxArray__common *)iindx, itmp, (int)sizeof(int));
      n = 0;
      for (ixstop = 0; ixstop <= end; ixstop++) {
        if (b_class->data[ixstop] == 0.0) {
          iindx->data[n] = ixstop + 1;
          n++;
        }
      }

      emxInit_boolean_T(&d_class, 1);
      itmp = d_class->size[0];
      d_class->size[0] = b_class->size[0];
      emxEnsureCapacity((emxArray__common *)d_class, itmp,
                        (int)sizeof(boolean_T));
      loop_ub = b_class->size[0];
      for (itmp = 0; itmp < loop_ub; itmp++) {
        d_class->data[itmp] = (b_class->data[itmp] == 0.0);
      }

      d0 = sum(d_class);
      loop_ub = iindx->size[0];
      emxFree_boolean_T(&d_class);
      for (itmp = 0; itmp < loop_ub; itmp++) {
        area->data[iindx->data[itmp] - 1] = d0;
      }

      vstride = data->size[0];
      itmp = w->size[0];
      w->size[0] = area->size[0];
      emxEnsureCapacity((emxArray__common *)w, itmp, (int)sizeof(double));
      loop_ub = area->size[0];
      for (itmp = 0; itmp < loop_ub; itmp++) {
        w->data[itmp] = 100.0 * area->data[itmp] / (double)vstride;
      }
    }

    emxFree_int32_T(&iindx);
    emxFree_int32_T(&c);
    emxFree_real_T(&C);
  }

  emxFree_real_T(&b_data);
  emxFree_real_T(&y);
  emxFree_real_T(&x);
  emxFree_real_T(&b_rand_eigval);
  emxFree_real_T(&b_sub_data);
  emxFree_real_T(&unusedU5);
  emxFree_real_T(&unusedU6);
  emxFree_real_T(&rand_eigval);
  emxFree_real_T(&sub_data);
}

//
// Arguments    : void
// Return Type  : int
//
int getThreadID() { return threadID; }

//
// Arguments    : void
// Return Type  : void
//
void getThreadID_init() {
  int ub_loop;
  int i;
  ub_loop = omp_get_max_threads();

#pragma omp parallel for schedule(static) num_threads(omp_get_max_threads())

  for (i = 1; i <= ub_loop; i++) {
    threadID = omp_get_thread_num();
  }
}

//
// Clustering algorithm based on principal component analysis (PCA) finds
// patterns in dataset.
//  Realizations with simmilar component are assigned to same class.
//  Non-significant points has zero class.
//
//
//  INPUT:
//      MA/MW... double amplitude/full_weight metrix [n x dim], n-number of
//      realization, dim-dimension
//      MW.*(MV>0)... double condition metrix [n x dim], n-number of
//      realization, dim-dimension
//      pca_centering... set centering of dataset MA:
//      MA=MA-repmat(pca_centering*mean(MA,1),size(MA,1),1);
//                   ... 0 - no centered, higher merging/ lower separation
//                   ... 1 - full centered
//
//  OUTPUTS:
//      cluster.class... matrix [n x 1], cluster assigment (>0). Zero value
//      marks unclassified realizations
//      cluster.weight ... matrix [n x 1], percentage of classes
//      cluster.area ... matrix [n x 1], number of realization in same classes
//
//  Made by ISARG 2014, Radek Janca (jancarad@fel.cvut.cz)
//
//  rozpracov�no !!!!!!!!!!!!!!!!!!!!!!!!!
// Arguments    : const emxArray_real_T *MA
//                const emxArray_real_T *MW
//                double pca_centering
//                struct0_T *cluster
// Return Type  : void
//
void spike_cluster_cpp(const emxArray_real_T *MA, const emxArray_real_T *MW,
                       double pca_centering, struct0_T *cluster) {
  int i0;
  int ixstart;
  boolean_T stack;
  emxArray_real_T *qEEG_MW;
  emxArray_uint32_T *significant_ch;
  emxArray_boolean_T *idx_evt;
  emxArray_real_T *poz;
  emxArray_real_T *out1;
  emxArray_real_T *out3;
  emxArray_boolean_T *r0;
  emxArray_boolean_T *r1;
  emxArray_int32_T *r2;
  emxArray_int32_T *iidx;
  emxArray_int32_T *ii;
  emxArray_boolean_T *b_cluster;
  emxArray_boolean_T *c_cluster;
  emxArray_real_T *b_MA;
  emxArray_boolean_T *d_cluster;
  emxArray_real_T *c_MA;
  emxArray_real_T *b_MW;
  emxArray_boolean_T *e_cluster;
  emxArray_int32_T *b_qEEG_MW;
  emxArray_real_T *f_cluster;
  double zero_length;
  int idx;
  int n;
  unsigned int b_index;
  double mtmp;
  int x_size[2];
  boolean_T exitg2;
  double x_data[1];
  boolean_T exitg1;
  boolean_T guard1 = false;
  static double dv0[375000];
  boolean_T b_guard1 = false;

  //  init
  i0 = cluster->b_class->size[0];
  cluster->b_class->size[0] = MA->size[0];
  emxEnsureCapacity((emxArray__common *)cluster->b_class, i0,
                    (int)sizeof(double));
  ixstart = MA->size[0];
  for (i0 = 0; i0 < ixstart; i0++) {
    cluster->b_class->data[i0] = 0.0;
  }

  i0 = cluster->weight->size[0];
  cluster->weight->size[0] = MA->size[0];
  emxEnsureCapacity((emxArray__common *)cluster->weight, i0,
                    (int)sizeof(double));
  ixstart = MA->size[0];
  for (i0 = 0; i0 < ixstart; i0++) {
    cluster->weight->data[i0] = 0.0;
  }

  i0 = cluster->area->size[0];
  cluster->area->size[0] = MA->size[0];
  emxEnsureCapacity((emxArray__common *)cluster->area, i0, (int)sizeof(double));
  ixstart = MA->size[0];
  for (i0 = 0; i0 < ixstart; i0++) {
    cluster->area->data[i0] = 0.0;
  }

  //  z-score norm.
  // fprintf(1,'events passed ')
  stack = true;
  emxInit_real_T(&qEEG_MW, 2);
  emxInit_uint32_T(&significant_ch, 2);
  emxInit_boolean_T(&idx_evt, 1);
  emxInit_real_T1(&poz, 1);
  emxInit_real_T1(&out1, 1);
  emxInit_real_T1(&out3, 1);
  emxInit_boolean_T(&r0, 1);
  emxInit_boolean_T(&r1, 1);
  emxInit_int32_T(&r2, 1);
  emxInit_int32_T1(&iidx, 2);
  emxInit_int32_T(&ii, 1);
  emxInit_boolean_T(&b_cluster, 1);
  emxInit_boolean_T(&c_cluster, 1);
  emxInit_real_T(&b_MA, 2);
  emxInit_boolean_T(&d_cluster, 1);
  emxInit_real_T(&c_MA, 2);
  emxInit_real_T(&b_MW, 2);
  emxInit_boolean_T(&e_cluster, 1);
  emxInit_int32_T1(&b_qEEG_MW, 2);
  emxInit_real_T1(&f_cluster, 1);
  while (stack) {
    i0 = e_cluster->size[0];
    e_cluster->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)e_cluster, i0,
                      (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      e_cluster->data[i0] = (cluster->b_class->data[i0] == 0.0);
    }

    zero_length = sum(e_cluster);

    // fprintf(1,[num2str(100*(1-zero_length/size(MA,1)),'% 04.0f') '%%'])
    // if exist('MW')>0
    // warning off all
    i0 = r1->size[0];
    r1->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)r1, i0, (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      r1->data[i0] = (cluster->b_class->data[i0] == 0.0);
    }

    idx = r1->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (r1->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (r1->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    ixstart = MW->size[1];
    i0 = b_MW->size[0] * b_MW->size[1];
    b_MW->size[0] = ii->size[0];
    b_MW->size[1] = ixstart;
    emxEnsureCapacity((emxArray__common *)b_MW, i0, (int)sizeof(double));
    for (i0 = 0; i0 < ixstart; i0++) {
      idx = ii->size[0];
      for (n = 0; n < idx; n++) {
        b_MW->data[n + b_MW->size[0] * i0] =
            MW->data[(ii->data[n] + MW->size[0] * i0) - 1];
      }
    }

    b_sum(b_MW, qEEG_MW);

    // try
    //     [cm,centr]=kmeans(qEEG_MW,2,'replicates',20);
    //     [~,poz]=max(centr);
    //     significant_ch=find(cm==poz);
    // catch
    //     try
    //         [cm,centr]=kmeans(qEEG_MW,2,'start',[min(qEEG_MW)
    //         ;max(qEEG_MW)]);
    //         [~,poz]=max(centr);
    //         significant_ch=find(cm==poz);
    //     catch
    //         significant_ch=1:size(MW,2);
    //     end
    // end
    // if size(qEEG_MW, 1) > 2
    //     [cm,centr]=kmeans(qEEG_MW,2,'replicates',20);
    //     [~,poz]=max(centr);
    //     significant_ch=find(cm==poz);
    // else
    //     startM = [min(qEEG_MW) ;max(qEEG_MW)];
    //     if size(startM, 2) == size(qEEG_MW, 2)
    //         [cm,centr]=kmeans(qEEG_MW,2,'start',startM);
    //         [~,poz]=max(centr);
    //         significant_ch=find(cm==poz);
    //     else
    if (MW->size[1] < 1) {
      i0 = significant_ch->size[0] * significant_ch->size[1];
      significant_ch->size[0] = 1;
      significant_ch->size[1] = 0;
      emxEnsureCapacity((emxArray__common *)significant_ch, i0,
                        (int)sizeof(unsigned int));
    } else {
      b_index = MW->size[1] + MAX_uint32_T;
      i0 = significant_ch->size[0] * significant_ch->size[1];
      significant_ch->size[0] = 1;
      significant_ch->size[1] = (int)b_index + 1;
      emxEnsureCapacity((emxArray__common *)significant_ch, i0,
                        (int)sizeof(unsigned int));
      ixstart = (int)b_index;
      for (i0 = 0; i0 <= ixstart; i0++) {
        significant_ch->data[significant_ch->size[0] * i0] = 1U + i0;
      }
    }

    //     end
    // end
    sort(qEEG_MW, iidx);
    i0 = qEEG_MW->size[0] * qEEG_MW->size[1];
    qEEG_MW->size[0] = 1;
    qEEG_MW->size[1] = iidx->size[1];
    emxEnsureCapacity((emxArray__common *)qEEG_MW, i0, (int)sizeof(double));
    ixstart = iidx->size[0] * iidx->size[1];
    for (i0 = 0; i0 < ixstart; i0++) {
      qEEG_MW->data[i0] = iidx->data[i0];
    }

    b_index = (unsigned int)significant_ch->size[1] << 1;
    if (b_index <= 1U) {
      if (1 > (int)b_index) {
        ixstart = -1;
      } else {
        ixstart = 0;
      }

      i0 = b_qEEG_MW->size[0] * b_qEEG_MW->size[1];
      b_qEEG_MW->size[0] = 1;
      b_qEEG_MW->size[1] = qEEG_MW->size[1];
      emxEnsureCapacity((emxArray__common *)b_qEEG_MW, i0, (int)sizeof(int));
      idx = qEEG_MW->size[1];
      for (i0 = 0; i0 < idx; i0++) {
        b_qEEG_MW->data[b_qEEG_MW->size[0] * i0] =
            (int)qEEG_MW->data[qEEG_MW->size[0] * i0];
      }

      x_size[0] = 1;
      x_size[1] = ixstart + 1;
      for (i0 = 0; i0 <= ixstart; i0++) {
        x_data[i0] = b_qEEG_MW->data[i0];
      }

      c_sort(x_data, x_size);
      i0 = qEEG_MW->size[0] * qEEG_MW->size[1];
      qEEG_MW->size[0] = 1;
      qEEG_MW->size[1] = x_size[1];
      emxEnsureCapacity((emxArray__common *)qEEG_MW, i0, (int)sizeof(double));
      ixstart = x_size[0] * x_size[1];
      for (i0 = 0; i0 < ixstart; i0++) {
        qEEG_MW->data[i0] = x_data[i0];
      }
    } else if (MW->size[1] < 1) {
      i0 = qEEG_MW->size[0] * qEEG_MW->size[1];
      qEEG_MW->size[0] = 1;
      qEEG_MW->size[1] = 0;
      emxEnsureCapacity((emxArray__common *)qEEG_MW, i0, (int)sizeof(double));
    } else {
      i0 = MW->size[1];
      n = qEEG_MW->size[0] * qEEG_MW->size[1];
      qEEG_MW->size[0] = 1;
      qEEG_MW->size[1] = (int)((double)i0 - 1.0) + 1;
      emxEnsureCapacity((emxArray__common *)qEEG_MW, n, (int)sizeof(double));
      ixstart = (int)((double)i0 - 1.0);
      for (i0 = 0; i0 <= ixstart; i0++) {
        qEEG_MW->data[qEEG_MW->size[0] * i0] = 1.0 + (double)i0;
      }
    }

    ixstart = 1;
    n = cluster->b_class->size[0];
    mtmp = cluster->b_class->data[0];
    if (cluster->b_class->size[0] > 1) {
      if (rtIsNaN(cluster->b_class->data[0])) {
        idx = 2;
        exitg2 = false;
        while ((!exitg2) && (idx <= n)) {
          ixstart = idx;
          if (!rtIsNaN(cluster->b_class->data[idx - 1])) {
            mtmp = cluster->b_class->data[idx - 1];
            exitg2 = true;
          } else {
            idx++;
          }
        }
      }

      if (ixstart < cluster->b_class->size[0]) {
        while (ixstart + 1 <= n) {
          if (cluster->b_class->data[ixstart] > mtmp) {
            mtmp = cluster->b_class->data[ixstart];
          }

          ixstart++;
        }
      }
    }

    i0 = idx_evt->size[0];
    idx_evt->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)idx_evt, i0, (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      idx_evt->data[i0] = (cluster->b_class->data[i0] == 0.0);
    }

    //      disp([num2str(sum(idx_evt)) ' events'])
    if (sum(idx_evt) > 500000.0) {
      n = idx_evt->size[0];
      idx = 0;
      i0 = ii->size[0];
      ii->size[0] = idx_evt->size[0];
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 1;
      exitg1 = false;
      while ((!exitg1) && (ixstart <= n)) {
        guard1 = false;
        if (idx_evt->data[ixstart - 1]) {
          idx++;
          ii->data[idx - 1] = ixstart;
          if (idx >= n) {
            exitg1 = true;
          } else {
            guard1 = true;
          }
        } else {
          guard1 = true;
        }

        if (guard1) {
          ixstart++;
        }
      }

      if (idx_evt->size[0] == 1) {
        if (idx == 0) {
          i0 = ii->size[0];
          ii->size[0] = 0;
          emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
        }
      } else {
        i0 = ii->size[0];
        if (1 > idx) {
          ii->size[0] = 0;
        } else {
          ii->size[0] = idx;
        }

        emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      }

      i0 = poz->size[0];
      poz->size[0] = ii->size[0];
      emxEnsureCapacity((emxArray__common *)poz, i0, (int)sizeof(double));
      ixstart = ii->size[0];
      for (i0 = 0; i0 < ixstart; i0++) {
        poz->data[i0] = ii->data[i0];
      }

      b_index = (unsigned int)idx_evt->size[0];
      i0 = idx_evt->size[0];
      idx_evt->size[0] = (int)b_index;
      emxEnsureCapacity((emxArray__common *)idx_evt, i0,
                        (int)sizeof(boolean_T));
      ixstart = (int)b_index;
      for (i0 = 0; i0 < ixstart; i0++) {
        idx_evt->data[i0] = false;
      }

      randperm((double)poz->size[0], dv0);
      for (i0 = 0; i0 < 375000; i0++) {
        idx_evt->data[(int)poz->data[(int)dv0[i0] - 1] - 1] = true;
      }
    }

    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    i0 = c_MA->size[0] * c_MA->size[1];
    c_MA->size[0] = ii->size[0];
    c_MA->size[1] = qEEG_MW->size[1];
    emxEnsureCapacity((emxArray__common *)c_MA, i0, (int)sizeof(double));
    ixstart = qEEG_MW->size[1];
    for (i0 = 0; i0 < ixstart; i0++) {
      idx = ii->size[0];
      for (n = 0; n < idx; n++) {
        c_MA->data[n + c_MA->size[0] * i0] =
            MA->data[(ii->data[n] +
                      MA->size[0] *
                          ((int)qEEG_MW->data[qEEG_MW->size[0] * i0] - 1)) -
                     1];
      }
    }

    spike_cluster_sub(c_MA, pca_centering, out1, poz, out3);

    // [cluster.class(idx_evt),~,cluster.area(idx_evt)]=spike_cluster_sub(MA(idx_evt,idx_ch),pca_centering);
    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    repeat_if_smaller(out1, (double)ii->size[0]);
    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    ixstart = out1->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      cluster->b_class->data[ii->data[i0] - 1] = out1->data[i0];
    }

    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    repeat_if_smaller(out3, (double)ii->size[0]);
    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    ixstart = out3->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      cluster->area->data[ii->data[i0] - 1] = out3->data[i0];
    }

    i0 = d_cluster->size[0];
    d_cluster->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)d_cluster, i0,
                      (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      d_cluster->data[i0] = (cluster->b_class->data[i0] == 0.0);
    }

    if (zero_length - sum(d_cluster) == 0.0) {
      // [cluster.class(idx_evt),~,cluster.area(idx_evt)]=spike_cluster_sub(MA(idx_evt,:),pca_centering);
      idx = idx_evt->size[0] - 1;
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ixstart++;
        }
      }

      i0 = ii->size[0];
      ii->size[0] = ixstart;
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ii->data[ixstart] = n + 1;
          ixstart++;
        }
      }

      ixstart = MA->size[1];
      i0 = b_MA->size[0] * b_MA->size[1];
      b_MA->size[0] = ii->size[0];
      b_MA->size[1] = ixstart;
      emxEnsureCapacity((emxArray__common *)b_MA, i0, (int)sizeof(double));
      for (i0 = 0; i0 < ixstart; i0++) {
        idx = ii->size[0];
        for (n = 0; n < idx; n++) {
          b_MA->data[n + b_MA->size[0] * i0] =
              MA->data[(ii->data[n] + MA->size[0] * i0) - 1];
        }
      }

      spike_cluster_sub(b_MA, pca_centering, out1, poz, out3);
      idx = idx_evt->size[0] - 1;
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ixstart++;
        }
      }

      i0 = ii->size[0];
      ii->size[0] = ixstart;
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ii->data[ixstart] = n + 1;
          ixstart++;
        }
      }

      repeat_if_smaller(out1, (double)ii->size[0]);
      idx = idx_evt->size[0] - 1;
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ixstart++;
        }
      }

      i0 = ii->size[0];
      ii->size[0] = ixstart;
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ii->data[ixstart] = n + 1;
          ixstart++;
        }
      }

      ixstart = out1->size[0];
      for (i0 = 0; i0 < ixstart; i0++) {
        cluster->b_class->data[ii->data[i0] - 1] = out1->data[i0];
      }

      idx = idx_evt->size[0] - 1;
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ixstart++;
        }
      }

      i0 = ii->size[0];
      ii->size[0] = ixstart;
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ii->data[ixstart] = n + 1;
          ixstart++;
        }
      }

      repeat_if_smaller(out3, (double)ii->size[0]);
      idx = idx_evt->size[0] - 1;
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ixstart++;
        }
      }

      i0 = ii->size[0];
      ii->size[0] = ixstart;
      emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
      ixstart = 0;
      for (n = 0; n <= idx; n++) {
        if (idx_evt->data[n]) {
          ii->data[ixstart] = n + 1;
          ixstart++;
        }
      }

      ixstart = out3->size[0];
      for (i0 = 0; i0 < ixstart; i0++) {
        cluster->area->data[ii->data[i0] - 1] = out3->data[i0];
      }
    }

    i0 = r1->size[0];
    r1->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)r1, i0, (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      r1->data[i0] = (cluster->b_class->data[i0] > 0.0);
    }

    i0 = r0->size[0];
    r0->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)r0, i0, (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      r0->data[i0] = (cluster->b_class->data[i0] > 0.0);
    }

    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n] && r0->data[n]) {
        ixstart++;
      }
    }

    i0 = ii->size[0];
    ii->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)ii, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n] && r0->data[n]) {
        ii->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    idx = idx_evt->size[0] - 1;
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n] && r1->data[n]) {
        ixstart++;
      }
    }

    i0 = r2->size[0];
    r2->size[0] = ixstart;
    emxEnsureCapacity((emxArray__common *)r2, i0, (int)sizeof(int));
    ixstart = 0;
    for (n = 0; n <= idx; n++) {
      if (idx_evt->data[n] && r1->data[n]) {
        r2->data[ixstart] = n + 1;
        ixstart++;
      }
    }

    i0 = f_cluster->size[0];
    f_cluster->size[0] = r2->size[0];
    emxEnsureCapacity((emxArray__common *)f_cluster, i0, (int)sizeof(double));
    ixstart = r2->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      f_cluster->data[i0] = cluster->b_class->data[r2->data[i0] - 1] + mtmp;
    }

    ixstart = f_cluster->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      cluster->b_class->data[ii->data[i0] - 1] = f_cluster->data[i0];
    }

    i0 = c_cluster->size[0];
    c_cluster->size[0] = cluster->b_class->size[0];
    emxEnsureCapacity((emxArray__common *)c_cluster, i0,
                      (int)sizeof(boolean_T));
    ixstart = cluster->b_class->size[0];
    for (i0 = 0; i0 < ixstart; i0++) {
      c_cluster->data[i0] = (cluster->b_class->data[i0] == 0.0);
    }

    b_guard1 = false;
    if (sum(c_cluster) == 0.0) {
      b_guard1 = true;
    } else {
      i0 = b_cluster->size[0];
      b_cluster->size[0] = cluster->b_class->size[0];
      emxEnsureCapacity((emxArray__common *)b_cluster, i0,
                        (int)sizeof(boolean_T));
      ixstart = cluster->b_class->size[0];
      for (i0 = 0; i0 < ixstart; i0++) {
        b_cluster->data[i0] = (cluster->b_class->data[i0] == 0.0);
      }

      if (zero_length - sum(b_cluster) == 0.0) {
        b_guard1 = true;
      }
    }

    if (b_guard1) {
      stack = false;
    }

    // fprintf(1,repmat('\b',1,4))
  }

  emxFree_real_T(&f_cluster);
  emxFree_int32_T(&b_qEEG_MW);
  emxFree_boolean_T(&e_cluster);
  emxFree_real_T(&b_MW);
  emxFree_real_T(&c_MA);
  emxFree_boolean_T(&d_cluster);
  emxFree_real_T(&b_MA);
  emxFree_boolean_T(&c_cluster);
  emxFree_boolean_T(&b_cluster);
  emxFree_int32_T(&ii);
  emxFree_int32_T(&iidx);
  emxFree_int32_T(&r2);
  emxFree_boolean_T(&r1);
  emxFree_boolean_T(&r0);
  emxFree_real_T(&out3);
  emxFree_real_T(&out1);
  emxFree_real_T(&poz);
  emxFree_boolean_T(&idx_evt);
  emxFree_uint32_T(&significant_ch);
  emxFree_real_T(&qEEG_MW);

  // fprintf(1,' done\n')
  cluster_merging(cluster, MW);
}
