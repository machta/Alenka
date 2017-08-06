
// Include Files
#include "randperm.h"
#include "mod.h"
#include "rand.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"
#include "spike_cluster_cpp_rtwutil.h"

// Function Declarations
static void b_randkperm(double n, double rndperm[50000]);
static void randkperm(double n, double rndperm[375000]);

// Function Definitions

//
// Arguments    : double n
//                double rndperm[50000]
// Return Type  : void
//
static void b_randkperm(double n, double rndperm[50000]) {
  unsigned short hashTbl[50000];
  double nleftm1;
  int m;
  double selectedLoc;
  double i;
  double newEntry;
  double j;
  spike_cluster_cppTLS *spike_cluster_cppTLSThread;
  spike_cluster_cppTLSThread = emlrtGetThreadStackData();
  memset(&rndperm[0], 0, 50000U * sizeof(double));
  if (50000.0 >= n / 4.0) {
    nleftm1 = 0.0;
    for (m = 0; m < 50000; m++) {
      selectedLoc = n - nleftm1;
      i = (50000.0 - (double)m) / selectedLoc;
      newEntry = b_rand();
      while (newEntry > i) {
        nleftm1++;
        selectedLoc--;
        i += (1.0 - i) * ((50000.0 - (double)m) / selectedLoc);
      }

      nleftm1++;
      selectedLoc = b_rand() * ((double)m + 1.0);
      j = std::floor(selectedLoc);
      rndperm[m] = rndperm[(int)(j + 1.0) - 1];
      rndperm[(int)(j + 1.0) - 1] = nleftm1;
    }
  } else {
    memset(&hashTbl[0], 0, 50000U * sizeof(unsigned short));
    memset(&spike_cluster_cppTLSThread->f0.link[0], 0, 50000U * sizeof(double));
    memset(&spike_cluster_cppTLSThread->f0.val[0], 0, 50000U * sizeof(double));
    memset(&spike_cluster_cppTLSThread->f0.loc[0], 0, 50000U * sizeof(double));
    newEntry = 1.0;
    for (m = 0; m < 50000; m++) {
      nleftm1 = n - (1.0 + (double)m);
      selectedLoc = b_rand() * (nleftm1 + 1.0);
      selectedLoc = std::floor(selectedLoc);
      i = 1.0 + b_mod(selectedLoc, 50000.0);
      j = hashTbl[(int)i - 1];
      while ((j > 0.0) &&
             (spike_cluster_cppTLSThread->f0.loc[(int)j - 1] != selectedLoc)) {
        j = spike_cluster_cppTLSThread->f0.link[(int)j - 1];
      }

      if (j > 0.0) {
        rndperm[m] = spike_cluster_cppTLSThread->f0.val[(int)j - 1] + 1.0;
      } else {
        rndperm[m] = selectedLoc + 1.0;
        j = newEntry;
        newEntry++;
        spike_cluster_cppTLSThread->f0.loc[(int)j - 1] = selectedLoc;
        spike_cluster_cppTLSThread->f0.link[(int)j - 1] = hashTbl[(int)i - 1];
        hashTbl[(int)i - 1] = (unsigned short)j;
      }

      if (1 + m < 50000) {
        selectedLoc = hashTbl[(int)(1.0 + b_mod(nleftm1, 50000.0)) - 1];
        while ((selectedLoc > 0.0) &&
               (spike_cluster_cppTLSThread->f0.loc[(int)selectedLoc - 1] !=
                nleftm1)) {
          selectedLoc =
              spike_cluster_cppTLSThread->f0.link[(int)selectedLoc - 1];
        }

        if (selectedLoc > 0.0) {
          spike_cluster_cppTLSThread->f0.val[(int)j - 1] =
              spike_cluster_cppTLSThread->f0.val[(int)selectedLoc - 1];
        } else {
          spike_cluster_cppTLSThread->f0.val[(int)j - 1] = nleftm1;
        }
      }
    }
  }
}

//
// Arguments    : double n
//                double rndperm[375000]
// Return Type  : void
//
static void randkperm(double n, double rndperm[375000]) {
  static int hashTbl[375000];
  double nleftm1;
  int m;
  static double link[375000];
  static double val[375000];
  double selectedLoc;
  static double loc[375000];
  double j;
  double i;
  double newEntry;
  memset(&rndperm[0], 0, 375000U * sizeof(double));
  if (375000.0 >= n) {
    rndperm[0] = 1.0;
    for (m = 0; m < 374999; m++) {
      selectedLoc = b_rand() * ((1.0 + (double)m) + 1.0);
      j = std::floor(selectedLoc);
      rndperm[1 + m] = rndperm[(int)(j + 1.0) - 1];
      rndperm[(int)(j + 1.0) - 1] = (1.0 + (double)m) + 1.0;
    }
  } else if (375000.0 >= n / 4.0) {
    nleftm1 = 0.0;
    for (m = 0; m < 375000; m++) {
      selectedLoc = n - nleftm1;
      i = (375000.0 - (double)m) / selectedLoc;
      newEntry = b_rand();
      while (newEntry > i) {
        nleftm1++;
        selectedLoc--;
        i += (1.0 - i) * ((375000.0 - (double)m) / selectedLoc);
      }

      nleftm1++;
      selectedLoc = b_rand() * ((double)m + 1.0);
      j = std::floor(selectedLoc);
      rndperm[m] = rndperm[(int)(j + 1.0) - 1];
      rndperm[(int)(j + 1.0) - 1] = nleftm1;
    }
  } else {
    memset(&hashTbl[0], 0, 375000U * sizeof(int));
    memset(&link[0], 0, 375000U * sizeof(double));
    memset(&val[0], 0, 375000U * sizeof(double));
    memset(&loc[0], 0, 375000U * sizeof(double));
    newEntry = 1.0;
    for (m = 0; m < 375000; m++) {
      nleftm1 = n - (1.0 + (double)m);
      selectedLoc = b_rand() * (nleftm1 + 1.0);
      selectedLoc = std::floor(selectedLoc);
      i = 1.0 + b_mod(selectedLoc, 375000.0);
      j = hashTbl[(int)i - 1];
      while ((j > 0.0) && (loc[(int)j - 1] != selectedLoc)) {
        j = link[(int)j - 1];
      }

      if (j > 0.0) {
        rndperm[m] = val[(int)j - 1] + 1.0;
      } else {
        rndperm[m] = selectedLoc + 1.0;
        j = newEntry;
        newEntry++;
        loc[(int)j - 1] = selectedLoc;
        link[(int)j - 1] = hashTbl[(int)i - 1];
        hashTbl[(int)i - 1] = (int)j;
      }

      if (1 + m < 375000) {
        selectedLoc = hashTbl[(int)(1.0 + b_mod(nleftm1, 375000.0)) - 1];
        while ((selectedLoc > 0.0) && (loc[(int)selectedLoc - 1] != nleftm1)) {
          selectedLoc = link[(int)selectedLoc - 1];
        }

        if (selectedLoc > 0.0) {
          val[(int)j - 1] = val[(int)selectedLoc - 1];
        } else {
          val[(int)j - 1] = nleftm1;
        }
      }
    }
  }
}

//
// Arguments    : double n
//                double p[50000]
// Return Type  : void
//
void b_randperm(double n, double p[50000]) { b_randkperm(n, p); }

//
// Arguments    : double n
//                emxArray_real_T *p
// Return Type  : void
//
void c_randperm(double n, emxArray_real_T *p) {
  emxArray_int32_T *idx;
  int b_n;
  int i;
  int loop_ub;
  emxArray_int32_T *iwork;
  int b_p[2];
  boolean_T c_p;
  int i2;
  int j;
  int pEnd;
  int d_p;
  int q;
  int qEnd;
  int kEnd;
  emxInit_int32_T1(&idx, 2);
  c_rand(n, p);
  b_n = p->size[1] + 1;
  i = idx->size[0] * idx->size[1];
  idx->size[0] = 1;
  idx->size[1] = p->size[1];
  emxEnsureCapacity((emxArray__common *)idx, i, (int)sizeof(int));
  loop_ub = p->size[1];
  for (i = 0; i < loop_ub; i++) {
    idx->data[i] = 0;
  }

  if (p->size[1] != 0) {
    emxInit_int32_T(&iwork, 1);
    loop_ub = p->size[1];
    i = iwork->size[0];
    iwork->size[0] = loop_ub;
    emxEnsureCapacity((emxArray__common *)iwork, i, (int)sizeof(int));
    for (loop_ub = 1; loop_ub <= b_n - 2; loop_ub += 2) {
      if ((p->data[loop_ub - 1] <= p->data[loop_ub]) ||
          rtIsNaN(p->data[loop_ub])) {
        c_p = true;
      } else {
        c_p = false;
      }

      if (c_p) {
        idx->data[loop_ub - 1] = loop_ub;
        idx->data[loop_ub] = loop_ub + 1;
      } else {
        idx->data[loop_ub - 1] = loop_ub + 1;
        idx->data[loop_ub] = loop_ub;
      }
    }

    if ((p->size[1] & 1) != 0) {
      idx->data[p->size[1] - 1] = p->size[1];
    }

    i = 2;
    while (i < b_n - 1) {
      i2 = i << 1;
      j = 1;
      for (pEnd = 1 + i; pEnd < b_n; pEnd = qEnd + i) {
        d_p = j;
        q = pEnd - 1;
        qEnd = j + i2;
        if (qEnd > b_n) {
          qEnd = b_n;
        }

        loop_ub = 0;
        kEnd = qEnd - j;
        while (loop_ub + 1 <= kEnd) {
          if ((p->data[idx->data[d_p - 1] - 1] <= p->data[idx->data[q] - 1]) ||
              rtIsNaN(p->data[idx->data[q] - 1])) {
            c_p = true;
          } else {
            c_p = false;
          }

          if (c_p) {
            iwork->data[loop_ub] = idx->data[d_p - 1];
            d_p++;
            if (d_p == pEnd) {
              while (q + 1 < qEnd) {
                loop_ub++;
                iwork->data[loop_ub] = idx->data[q];
                q++;
              }
            }
          } else {
            iwork->data[loop_ub] = idx->data[q];
            q++;
            if (q + 1 == qEnd) {
              while (d_p < pEnd) {
                loop_ub++;
                iwork->data[loop_ub] = idx->data[d_p - 1];
                d_p++;
              }
            }
          }

          loop_ub++;
        }

        for (loop_ub = 0; loop_ub + 1 <= kEnd; loop_ub++) {
          idx->data[(j + loop_ub) - 1] = iwork->data[loop_ub];
        }

        j = qEnd;
      }

      i = i2;
    }

    emxFree_int32_T(&iwork);
  }

  for (i = 0; i < 2; i++) {
    b_p[i] = p->size[i];
  }

  i = p->size[0] * p->size[1];
  p->size[0] = 1;
  p->size[1] = b_p[1];
  emxEnsureCapacity((emxArray__common *)p, i, (int)sizeof(double));
  loop_ub = b_p[1];
  for (i = 0; i < loop_ub; i++) {
    p->data[p->size[0] * i] = idx->data[b_p[0] * i];
  }

  emxFree_int32_T(&idx);
}

//
// Arguments    : double n
//                double p[375000]
// Return Type  : void
//
void randperm(double n, double p[375000]) { randkperm(n, p); }
