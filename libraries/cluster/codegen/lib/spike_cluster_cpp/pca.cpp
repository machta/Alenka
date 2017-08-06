
// Include Files
#include "pca.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"
#include "spike_cluster_cpp_emxutil.h"
#include "sqrt.h"
#include "sum.h"
#include "xaxpy.h"
#include "xdotc.h"
#include "xnrm2.h"
#include "xrot.h"
#include "xrotg.h"
#include "xscal.h"
#include "xswap.h"

// Function Declarations
static void b_localSVD(emxArray_real_T *x, int DOF,
                       const emxArray_real_T *Weights,
                       const emxArray_real_T *VariableWeights,
                       emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                       emxArray_real_T *latentOut, emxArray_real_T *tsquared,
                       emxArray_real_T *explained);
static void localSVD(const emxArray_real_T *x, int DOF,
                     const emxArray_real_T *Weights,
                     const emxArray_real_T *VariableWeights,
                     emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                     emxArray_real_T *latentOut, emxArray_real_T *tsquared,
                     emxArray_real_T *explained);
static void localTSquared(const emxArray_real_T *score,
                          const emxArray_real_T *latent, int DOF, int p,
                          emxArray_real_T *tsquared);
static void local_pca(emxArray_real_T *x, int NumComponents,
                      const emxArray_real_T *Weights,
                      const emxArray_real_T *VariableWeights,
                      emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                      emxArray_real_T *latent);

// Function Definitions

//
// Arguments    : emxArray_real_T *x
//                int DOF
//                const emxArray_real_T *Weights
//                const emxArray_real_T *VariableWeights
//                emxArray_real_T *coeffOut
//                emxArray_real_T *scoreOut
//                emxArray_real_T *latentOut
//                emxArray_real_T *tsquared
//                emxArray_real_T *explained
// Return Type  : void
//
static void b_localSVD(emxArray_real_T *x, int DOF,
                       const emxArray_real_T *Weights,
                       const emxArray_real_T *VariableWeights,
                       emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                       emxArray_real_T *latentOut, emxArray_real_T *tsquared,
                       emxArray_real_T *explained) {
  emxArray_real_T *OmegaSqrt;
  int m;
  int qs;
  emxArray_real_T *PhiSqrt;
  int nrows;
  int ncols;
  emxArray_real_T *A;
  int iter;
  int n;
  int p;
  int ns;
  int minnp;
  emxArray_real_T *s;
  emxArray_real_T *e;
  emxArray_real_T *work;
  emxArray_real_T *U;
  emxArray_real_T *Vf;
  int nrt;
  int nct;
  int q;
  int nmq;
  boolean_T apply_transform;
  double ztest0;
  int jj;
  emxArray_real_T *coeff;
  double ztest;
  double snorm;
  boolean_T exitg3;
  boolean_T exitg2;
  double f;
  double varargin_1[5];
  double mtmp;
  boolean_T exitg1;
  double sqds;
  double b;
  emxInit_real_T1(&OmegaSqrt, 1);
  m = OmegaSqrt->size[0];
  OmegaSqrt->size[0] = Weights->size[0];
  emxEnsureCapacity((emxArray__common *)OmegaSqrt, m, (int)sizeof(double));
  qs = Weights->size[0];
  for (m = 0; m < qs; m++) {
    OmegaSqrt->data[m] = Weights->data[m];
  }

  emxInit_real_T(&PhiSqrt, 2);
  c_sqrt(OmegaSqrt);
  m = PhiSqrt->size[0] * PhiSqrt->size[1];
  PhiSqrt->size[0] = 1;
  PhiSqrt->size[1] = VariableWeights->size[1];
  emxEnsureCapacity((emxArray__common *)PhiSqrt, m, (int)sizeof(double));
  qs = VariableWeights->size[0] * VariableWeights->size[1];
  for (m = 0; m < qs; m++) {
    PhiSqrt->data[m] = VariableWeights->data[m];
  }

  b_sqrt(PhiSqrt);
  nrows = x->size[0];
  ncols = x->size[1];
  qs = 0;
  emxFree_real_T(&PhiSqrt);
  while (qs + 1 <= ncols) {
    for (iter = 0; iter + 1 <= nrows; iter++) {
      x->data[iter + x->size[0] * qs] *= OmegaSqrt->data[iter];
    }

    qs++;
  }

  emxInit_real_T(&A, 2);
  m = A->size[0] * A->size[1];
  A->size[0] = x->size[0];
  A->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)A, m, (int)sizeof(double));
  qs = x->size[0] * x->size[1];
  for (m = 0; m < qs; m++) {
    A->data[m] = x->data[m];
  }

  n = x->size[0];
  p = x->size[1];
  if (x->size[0] + 1 <= x->size[1]) {
    ns = x->size[0] + 1;
  } else {
    ns = x->size[1];
  }

  if (x->size[0] <= x->size[1]) {
    minnp = x->size[0];
  } else {
    minnp = x->size[1];
  }

  emxInit_real_T1(&s, 1);
  m = s->size[0];
  s->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)s, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    s->data[m] = 0.0;
  }

  emxInit_real_T1(&e, 1);
  ns = x->size[1];
  m = e->size[0];
  e->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)e, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    e->data[m] = 0.0;
  }

  emxInit_real_T1(&work, 1);
  ns = x->size[0];
  m = work->size[0];
  work->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)work, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    work->data[m] = 0.0;
  }

  emxInit_real_T(&U, 2);
  ns = x->size[0];
  m = U->size[0] * U->size[1];
  U->size[0] = ns;
  U->size[1] = minnp;
  emxEnsureCapacity((emxArray__common *)U, m, (int)sizeof(double));
  qs = ns * minnp;
  for (m = 0; m < qs; m++) {
    U->data[m] = 0.0;
  }

  emxInit_real_T(&Vf, 2);
  ns = x->size[1];
  qs = x->size[1];
  m = Vf->size[0] * Vf->size[1];
  Vf->size[0] = ns;
  Vf->size[1] = qs;
  emxEnsureCapacity((emxArray__common *)Vf, m, (int)sizeof(double));
  qs *= ns;
  for (m = 0; m < qs; m++) {
    Vf->data[m] = 0.0;
  }

  if ((x->size[0] == 0) || (x->size[1] == 0)) {
    if (x->size[0] <= minnp) {
      m = x->size[0];
    } else {
      m = minnp;
    }

    for (ns = 0; ns + 1 <= m; ns++) {
      U->data[ns + U->size[0] * ns] = 1.0;
    }

    m = x->size[1];
    for (ns = 0; ns + 1 <= m; ns++) {
      Vf->data[ns + Vf->size[0] * ns] = 1.0;
    }
  } else {
    if (x->size[1] > 2) {
      ns = x->size[1] - 2;
    } else {
      ns = 0;
    }

    if (ns <= x->size[0]) {
      nrt = ns;
    } else {
      nrt = x->size[0];
    }

    if (x->size[0] > 1) {
      ns = x->size[0] - 1;
    } else {
      ns = 0;
    }

    if (ns <= x->size[1]) {
      nct = ns;
    } else {
      nct = x->size[1];
    }

    if (nct >= nrt) {
      m = nct;
    } else {
      m = nrt;
    }

    for (q = 0; q + 1 <= m; q++) {
      iter = (q + n * q) + 1;
      nmq = n - q;
      apply_transform = false;
      if (q + 1 <= nct) {
        ztest0 = xnrm2(nmq, A, iter);
        if (ztest0 > 0.0) {
          apply_transform = true;
          if (A->data[iter - 1] < 0.0) {
            ztest0 = -ztest0;
          }

          s->data[q] = ztest0;
          if (std::abs(s->data[q]) >= 1.0020841800044864E-292) {
            xscal(nmq, 1.0 / s->data[q], A, iter);
          } else {
            ns = iter + nmq;
            for (qs = iter; qs < ns; qs++) {
              A->data[qs - 1] /= s->data[q];
            }
          }

          A->data[iter - 1]++;
          s->data[q] = -s->data[q];
        } else {
          s->data[q] = 0.0;
        }
      }

      for (jj = q + 1; jj + 1 <= p; jj++) {
        ns = q + n * jj;
        if (apply_transform) {
          ztest0 = xdotc(nmq, A, iter, A, ns + 1);
          ztest0 = -(ztest0 / A->data[q + A->size[0] * q]);
          xaxpy(nmq, ztest0, iter, A, ns + 1);
        }

        e->data[jj] = A->data[ns];
      }

      if (q + 1 <= nct) {
        for (ns = q; ns + 1 <= n; ns++) {
          U->data[ns + U->size[0] * q] = A->data[ns + A->size[0] * q];
        }
      }

      if (q + 1 <= nrt) {
        qs = (p - q) - 1;
        ztest0 = b_xnrm2(qs, e, q + 2);
        if (ztest0 == 0.0) {
          e->data[q] = 0.0;
        } else {
          if (e->data[q + 1] < 0.0) {
            ztest0 = -ztest0;
          }

          e->data[q] = ztest0;
          ztest0 = e->data[q];
          if (std::abs(ztest0) >= 1.0020841800044864E-292) {
            ztest0 = 1.0 / ztest0;
            ns = q + qs;
            for (qs = q + 1; qs + 1 <= ns + 1; qs++) {
              e->data[qs] *= ztest0;
            }
          } else {
            ns = q + qs;
            for (qs = q + 1; qs + 1 <= ns + 1; qs++) {
              e->data[qs] /= ztest0;
            }
          }

          e->data[q + 1]++;
          e->data[q] = -e->data[q];
          if (q + 2 <= n) {
            for (ns = q + 1; ns + 1 <= n; ns++) {
              work->data[ns] = 0.0;
            }

            for (jj = q + 1; jj + 1 <= p; jj++) {
              b_xaxpy(nmq - 1, e->data[jj], A, (q + n * jj) + 2, work, q + 2);
            }

            for (jj = q + 1; jj + 1 <= p; jj++) {
              c_xaxpy(nmq - 1, -e->data[jj] / e->data[q + 1], work, q + 2, A,
                      (q + n * jj) + 2);
            }
          }
        }

        for (ns = q + 1; ns + 1 <= p; ns++) {
          Vf->data[ns + Vf->size[0] * q] = e->data[ns];
        }
      }
    }

    if (x->size[1] <= x->size[0] + 1) {
      m = x->size[1];
    } else {
      m = x->size[0] + 1;
    }

    if (nct < x->size[1]) {
      s->data[nct] = A->data[nct + A->size[0] * nct];
    }

    if (x->size[0] < m) {
      s->data[m - 1] = 0.0;
    }

    if (nrt + 1 < m) {
      e->data[nrt] = A->data[nrt + A->size[0] * (m - 1)];
    }

    e->data[m - 1] = 0.0;
    if (nct + 1 <= minnp) {
      for (jj = nct; jj + 1 <= minnp; jj++) {
        for (ns = 1; ns <= n; ns++) {
          U->data[(ns + U->size[0] * jj) - 1] = 0.0;
        }

        U->data[jj + U->size[0] * jj] = 1.0;
      }
    }

    for (q = nct - 1; q + 1 > 0; q--) {
      nmq = n - q;
      iter = q + n * q;
      if (s->data[q] != 0.0) {
        for (jj = q; jj + 2 <= minnp; jj++) {
          ns = (q + n * (jj + 1)) + 1;
          ztest0 = xdotc(nmq, U, iter + 1, U, ns);
          ztest0 = -(ztest0 / U->data[iter]);
          xaxpy(nmq, ztest0, iter + 1, U, ns);
        }

        for (ns = q; ns + 1 <= n; ns++) {
          U->data[ns + U->size[0] * q] = -U->data[ns + U->size[0] * q];
        }

        U->data[iter]++;
        for (ns = 1; ns <= q; ns++) {
          U->data[(ns + U->size[0] * q) - 1] = 0.0;
        }
      } else {
        for (ns = 1; ns <= n; ns++) {
          U->data[(ns + U->size[0] * q) - 1] = 0.0;
        }

        U->data[iter] = 1.0;
      }
    }

    for (q = x->size[1] - 1; q + 1 > 0; q--) {
      if ((q + 1 <= nrt) && (e->data[q] != 0.0)) {
        qs = (p - q) - 1;
        ns = (q + p * q) + 2;
        for (jj = q; jj + 2 <= p; jj++) {
          iter = (q + p * (jj + 1)) + 2;
          ztest0 = xdotc(qs, Vf, ns, Vf, iter);
          ztest0 = -(ztest0 / Vf->data[ns - 1]);
          xaxpy(qs, ztest0, ns, Vf, iter);
        }
      }

      for (ns = 1; ns <= p; ns++) {
        Vf->data[(ns + Vf->size[0] * q) - 1] = 0.0;
      }

      Vf->data[q + Vf->size[0] * q] = 1.0;
    }

    for (q = 0; q + 1 <= m; q++) {
      if (s->data[q] != 0.0) {
        ztest = std::abs(s->data[q]);
        ztest0 = s->data[q] / ztest;
        s->data[q] = ztest;
        if (q + 1 < m) {
          e->data[q] /= ztest0;
        }

        if (q + 1 <= n) {
          xscal(n, ztest0, U, 1 + n * q);
        }
      }

      if ((q + 1 < m) && (e->data[q] != 0.0)) {
        ztest = std::abs(e->data[q]);
        ztest0 = ztest / e->data[q];
        e->data[q] = ztest;
        s->data[q + 1] *= ztest0;
        xscal(p, ztest0, Vf, 1 + p * (q + 1));
      }
    }

    nct = m;
    iter = 0;
    snorm = 0.0;
    for (ns = 0; ns + 1 <= m; ns++) {
      ztest0 = std::abs(s->data[ns]);
      ztest = std::abs(e->data[ns]);
      if ((ztest0 >= ztest) || rtIsNaN(ztest)) {
      } else {
        ztest0 = ztest;
      }

      if (!((snorm >= ztest0) || rtIsNaN(ztest0))) {
        snorm = ztest0;
      }
    }

    while ((m > 0) && (!(iter >= 75))) {
      q = m - 1;
      exitg3 = false;
      while (!(exitg3 || (q == 0))) {
        ztest0 = std::abs(e->data[q - 1]);
        if ((ztest0 <= 2.2204460492503131E-16 *
                           (std::abs(s->data[q - 1]) + std::abs(s->data[q]))) ||
            (ztest0 <= 1.0020841800044864E-292) ||
            ((iter > 20) && (ztest0 <= 2.2204460492503131E-16 * snorm))) {
          e->data[q - 1] = 0.0;
          exitg3 = true;
        } else {
          q--;
        }
      }

      if (q == m - 1) {
        ns = 4;
      } else {
        qs = m;
        ns = m;
        exitg2 = false;
        while ((!exitg2) && (ns >= q)) {
          qs = ns;
          if (ns == q) {
            exitg2 = true;
          } else {
            ztest0 = 0.0;
            if (ns < m) {
              ztest0 = std::abs(e->data[ns - 1]);
            }

            if (ns > q + 1) {
              ztest0 += std::abs(e->data[ns - 2]);
            }

            ztest = std::abs(s->data[ns - 1]);
            if ((ztest <= 2.2204460492503131E-16 * ztest0) ||
                (ztest <= 1.0020841800044864E-292)) {
              s->data[ns - 1] = 0.0;
              exitg2 = true;
            } else {
              ns--;
            }
          }
        }

        if (qs == q) {
          ns = 3;
        } else if (qs == m) {
          ns = 1;
        } else {
          ns = 2;
          q = qs;
        }
      }

      switch (ns) {
      case 1:
        f = e->data[m - 2];
        e->data[m - 2] = 0.0;
        for (qs = m - 3; qs + 2 >= q + 1; qs--) {
          xrotg(&s->data[qs + 1], &f, &ztest0, &ztest);
          if (qs + 2 > q + 1) {
            f = -ztest * e->data[qs];
            e->data[qs] *= ztest0;
          }

          xrot(p, Vf, 1 + p * (qs + 1), 1 + p * (m - 1), ztest0, ztest);
        }
        break;

      case 2:
        f = e->data[q - 1];
        e->data[q - 1] = 0.0;
        for (qs = q; qs + 1 <= m; qs++) {
          xrotg(&s->data[qs], &f, &ztest0, &ztest);
          f = -ztest * e->data[qs];
          e->data[qs] *= ztest0;
          xrot(n, U, 1 + n * qs, 1 + n * (q - 1), ztest0, ztest);
        }
        break;

      case 3:
        varargin_1[0] = std::abs(s->data[m - 1]);
        varargin_1[1] = std::abs(s->data[m - 2]);
        varargin_1[2] = std::abs(e->data[m - 2]);
        varargin_1[3] = std::abs(s->data[q]);
        varargin_1[4] = std::abs(e->data[q]);
        ns = 1;
        mtmp = varargin_1[0];
        if (rtIsNaN(varargin_1[0])) {
          qs = 2;
          exitg1 = false;
          while ((!exitg1) && (qs < 6)) {
            ns = qs;
            if (!rtIsNaN(varargin_1[qs - 1])) {
              mtmp = varargin_1[qs - 1];
              exitg1 = true;
            } else {
              qs++;
            }
          }
        }

        if (ns < 5) {
          while (ns + 1 < 6) {
            if (varargin_1[ns] > mtmp) {
              mtmp = varargin_1[ns];
            }

            ns++;
          }
        }

        f = s->data[m - 1] / mtmp;
        ztest0 = s->data[m - 2] / mtmp;
        ztest = e->data[m - 2] / mtmp;
        sqds = s->data[q] / mtmp;
        b = ((ztest0 + f) * (ztest0 - f) + ztest * ztest) / 2.0;
        ztest0 = f * ztest;
        ztest0 *= ztest0;
        if ((b != 0.0) || (ztest0 != 0.0)) {
          ztest = std::sqrt(b * b + ztest0);
          if (b < 0.0) {
            ztest = -ztest;
          }

          ztest = ztest0 / (b + ztest);
        } else {
          ztest = 0.0;
        }

        f = (sqds + f) * (sqds - f) + ztest;
        b = sqds * (e->data[q] / mtmp);
        for (qs = q + 1; qs < m; qs++) {
          xrotg(&f, &b, &ztest0, &ztest);
          if (qs > q + 1) {
            e->data[qs - 2] = f;
          }

          f = ztest0 * s->data[qs - 1] + ztest * e->data[qs - 1];
          e->data[qs - 1] = ztest0 * e->data[qs - 1] - ztest * s->data[qs - 1];
          b = ztest * s->data[qs];
          s->data[qs] *= ztest0;
          xrot(p, Vf, 1 + p * (qs - 1), 1 + p * qs, ztest0, ztest);
          s->data[qs - 1] = f;
          xrotg(&s->data[qs - 1], &b, &ztest0, &ztest);
          f = ztest0 * e->data[qs - 1] + ztest * s->data[qs];
          s->data[qs] = -ztest * e->data[qs - 1] + ztest0 * s->data[qs];
          b = ztest * e->data[qs];
          e->data[qs] *= ztest0;
          if (qs < n) {
            xrot(n, U, 1 + n * (qs - 1), 1 + n * qs, ztest0, ztest);
          }
        }

        e->data[m - 2] = f;
        iter++;
        break;

      default:
        if (s->data[q] < 0.0) {
          s->data[q] = -s->data[q];
          xscal(p, -1.0, Vf, 1 + p * q);
        }

        ns = q + 1;
        while ((q + 1 < nct) && (s->data[q] < s->data[ns])) {
          ztest = s->data[q];
          s->data[q] = s->data[ns];
          s->data[ns] = ztest;
          if (q + 1 < p) {
            xswap(p, Vf, 1 + p * q, 1 + p * (q + 1));
          }

          if (q + 1 < n) {
            xswap(n, U, 1 + n * q, 1 + n * (q + 1));
          }

          q = ns;
          ns++;
        }

        iter = 0;
        m--;
        break;
      }
    }
  }

  emxFree_real_T(&work);
  m = e->size[0];
  e->size[0] = minnp;
  emxEnsureCapacity((emxArray__common *)e, m, (int)sizeof(double));
  for (qs = 0; qs + 1 <= minnp; qs++) {
    e->data[qs] = s->data[qs];
  }

  emxFree_real_T(&s);
  ns = x->size[1];
  m = A->size[0] * A->size[1];
  A->size[0] = ns;
  A->size[1] = minnp;
  emxEnsureCapacity((emxArray__common *)A, m, (int)sizeof(double));
  for (qs = 0; qs + 1 <= minnp; qs++) {
    for (iter = 0; iter + 1 <= p; iter++) {
      A->data[iter + A->size[0] * qs] = Vf->data[iter + Vf->size[0] * qs];
    }
  }

  m = Vf->size[0] * Vf->size[1];
  Vf->size[0] = U->size[0];
  Vf->size[1] = U->size[1];
  emxEnsureCapacity((emxArray__common *)Vf, m, (int)sizeof(double));
  qs = U->size[0] * U->size[1];
  for (m = 0; m < qs; m++) {
    Vf->data[m] = U->data[m];
  }

  emxInit_real_T(&coeff, 2);
  m = coeff->size[0] * coeff->size[1];
  coeff->size[0] = A->size[0];
  coeff->size[1] = A->size[1];
  emxEnsureCapacity((emxArray__common *)coeff, m, (int)sizeof(double));
  qs = A->size[0] * A->size[1];
  for (m = 0; m < qs; m++) {
    coeff->data[m] = A->data[m];
  }

  ns = U->size[1];
  for (qs = 0; qs + 1 <= ns; qs++) {
    for (iter = 0; iter + 1 <= nrows; iter++) {
      Vf->data[iter + Vf->size[0] * qs] = Vf->data[iter + Vf->size[0] * qs] /
                                          OmegaSqrt->data[iter] * e->data[qs];
    }
  }

  emxFree_real_T(&OmegaSqrt);
  emxFree_real_T(&A);
  for (qs = 0; qs + 1 <= ns; qs++) {
    e->data[qs] = e->data[qs] * e->data[qs] / (double)DOF;
  }

  localTSquared(Vf, e, DOF, ncols, tsquared);
  if (DOF < ncols) {
    ns = U->size[1];
    if (DOF <= ns) {
      ns = DOF;
    }

    m = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = nrows;
    scoreOut->size[1] = ns;
    emxEnsureCapacity((emxArray__common *)scoreOut, m, (int)sizeof(double));
    qs = nrows * ns;
    for (m = 0; m < qs; m++) {
      scoreOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      for (iter = 0; iter + 1 <= nrows; iter++) {
        scoreOut->data[iter + scoreOut->size[0] * qs] =
            Vf->data[iter + Vf->size[0] * qs];
      }
    }

    m = latentOut->size[0];
    latentOut->size[0] = ns;
    emxEnsureCapacity((emxArray__common *)latentOut, m, (int)sizeof(double));
    for (m = 0; m < ns; m++) {
      latentOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      latentOut->data[qs] = e->data[qs];
    }

    m = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = ncols;
    coeffOut->size[1] = ns;
    emxEnsureCapacity((emxArray__common *)coeffOut, m, (int)sizeof(double));
    qs = ncols * ns;
    for (m = 0; m < qs; m++) {
      coeffOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      for (iter = 0; iter + 1 <= ncols; iter++) {
        coeffOut->data[iter + coeffOut->size[0] * qs] =
            coeff->data[iter + coeff->size[0] * qs];
      }
    }
  } else {
    m = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = Vf->size[0];
    scoreOut->size[1] = Vf->size[1];
    emxEnsureCapacity((emxArray__common *)scoreOut, m, (int)sizeof(double));
    qs = Vf->size[0] * Vf->size[1];
    for (m = 0; m < qs; m++) {
      scoreOut->data[m] = Vf->data[m];
    }

    m = latentOut->size[0];
    latentOut->size[0] = e->size[0];
    emxEnsureCapacity((emxArray__common *)latentOut, m, (int)sizeof(double));
    qs = e->size[0];
    for (m = 0; m < qs; m++) {
      latentOut->data[m] = e->data[m];
    }

    m = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = coeff->size[0];
    coeffOut->size[1] = coeff->size[1];
    emxEnsureCapacity((emxArray__common *)coeffOut, m, (int)sizeof(double));
    qs = coeff->size[0] * coeff->size[1];
    for (m = 0; m < qs; m++) {
      coeffOut->data[m] = coeff->data[m];
    }
  }

  emxFree_real_T(&Vf);
  emxFree_real_T(&e);
  emxFree_real_T(&U);
  emxFree_real_T(&coeff);
  ztest0 = d_sum(latentOut);
  m = explained->size[0];
  explained->size[0] = latentOut->size[0];
  emxEnsureCapacity((emxArray__common *)explained, m, (int)sizeof(double));
  qs = latentOut->size[0];
  for (m = 0; m < qs; m++) {
    explained->data[m] = 100.0 * latentOut->data[m] / ztest0;
  }
}

//
// Arguments    : const emxArray_real_T *x
//                int DOF
//                const emxArray_real_T *Weights
//                const emxArray_real_T *VariableWeights
//                emxArray_real_T *coeffOut
//                emxArray_real_T *scoreOut
//                emxArray_real_T *latentOut
//                emxArray_real_T *tsquared
//                emxArray_real_T *explained
// Return Type  : void
//
static void localSVD(const emxArray_real_T *x, int DOF,
                     const emxArray_real_T *Weights,
                     const emxArray_real_T *VariableWeights,
                     emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                     emxArray_real_T *latentOut, emxArray_real_T *tsquared,
                     emxArray_real_T *explained) {
  emxArray_real_T *OmegaSqrt;
  int m;
  int qs;
  emxArray_real_T *PhiSqrt;
  int nrows;
  int ncols;
  emxArray_real_T *A;
  int n;
  int p;
  int ns;
  int minnp;
  emxArray_real_T *s;
  emxArray_real_T *e;
  emxArray_real_T *work;
  emxArray_real_T *U;
  emxArray_real_T *Vf;
  int nrt;
  int nct;
  int q;
  int iter;
  int nmq;
  boolean_T apply_transform;
  double ztest0;
  int jj;
  emxArray_real_T *coeff;
  double ztest;
  double snorm;
  boolean_T exitg3;
  boolean_T exitg2;
  double f;
  double varargin_1[5];
  double mtmp;
  boolean_T exitg1;
  double sqds;
  double b;
  emxInit_real_T(&OmegaSqrt, 2);
  m = OmegaSqrt->size[0] * OmegaSqrt->size[1];
  OmegaSqrt->size[0] = 1;
  OmegaSqrt->size[1] = Weights->size[1];
  emxEnsureCapacity((emxArray__common *)OmegaSqrt, m, (int)sizeof(double));
  qs = Weights->size[0] * Weights->size[1];
  for (m = 0; m < qs; m++) {
    OmegaSqrt->data[m] = Weights->data[m];
  }

  emxInit_real_T(&PhiSqrt, 2);
  b_sqrt(OmegaSqrt);
  m = PhiSqrt->size[0] * PhiSqrt->size[1];
  PhiSqrt->size[0] = 1;
  PhiSqrt->size[1] = VariableWeights->size[1];
  emxEnsureCapacity((emxArray__common *)PhiSqrt, m, (int)sizeof(double));
  qs = VariableWeights->size[0] * VariableWeights->size[1];
  emxFree_real_T(&OmegaSqrt);
  for (m = 0; m < qs; m++) {
    PhiSqrt->data[m] = VariableWeights->data[m];
  }

  b_sqrt(PhiSqrt);
  nrows = x->size[0];
  ncols = x->size[1];
  emxFree_real_T(&PhiSqrt);
  emxInit_real_T(&A, 2);
  m = A->size[0] * A->size[1];
  A->size[0] = x->size[0];
  A->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)A, m, (int)sizeof(double));
  qs = x->size[0] * x->size[1];
  for (m = 0; m < qs; m++) {
    A->data[m] = x->data[m];
  }

  n = x->size[0];
  p = x->size[1];
  if (x->size[0] + 1 <= x->size[1]) {
    ns = x->size[0] + 1;
  } else {
    ns = x->size[1];
  }

  if (x->size[0] <= x->size[1]) {
    minnp = x->size[0];
  } else {
    minnp = x->size[1];
  }

  emxInit_real_T1(&s, 1);
  m = s->size[0];
  s->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)s, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    s->data[m] = 0.0;
  }

  emxInit_real_T1(&e, 1);
  ns = x->size[1];
  m = e->size[0];
  e->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)e, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    e->data[m] = 0.0;
  }

  emxInit_real_T1(&work, 1);
  ns = x->size[0];
  m = work->size[0];
  work->size[0] = ns;
  emxEnsureCapacity((emxArray__common *)work, m, (int)sizeof(double));
  for (m = 0; m < ns; m++) {
    work->data[m] = 0.0;
  }

  emxInit_real_T(&U, 2);
  ns = x->size[0];
  m = U->size[0] * U->size[1];
  U->size[0] = ns;
  U->size[1] = minnp;
  emxEnsureCapacity((emxArray__common *)U, m, (int)sizeof(double));
  qs = ns * minnp;
  for (m = 0; m < qs; m++) {
    U->data[m] = 0.0;
  }

  emxInit_real_T(&Vf, 2);
  ns = x->size[1];
  qs = x->size[1];
  m = Vf->size[0] * Vf->size[1];
  Vf->size[0] = ns;
  Vf->size[1] = qs;
  emxEnsureCapacity((emxArray__common *)Vf, m, (int)sizeof(double));
  qs *= ns;
  for (m = 0; m < qs; m++) {
    Vf->data[m] = 0.0;
  }

  if ((x->size[0] == 0) || (x->size[1] == 0)) {
    if (x->size[0] <= minnp) {
      m = x->size[0];
    } else {
      m = minnp;
    }

    for (ns = 0; ns + 1 <= m; ns++) {
      U->data[ns + U->size[0] * ns] = 1.0;
    }

    m = x->size[1];
    for (ns = 0; ns + 1 <= m; ns++) {
      Vf->data[ns + Vf->size[0] * ns] = 1.0;
    }
  } else {
    if (x->size[1] > 2) {
      ns = x->size[1] - 2;
    } else {
      ns = 0;
    }

    if (ns <= x->size[0]) {
      nrt = ns;
    } else {
      nrt = x->size[0];
    }

    if (x->size[0] > 1) {
      ns = x->size[0] - 1;
    } else {
      ns = 0;
    }

    if (ns <= x->size[1]) {
      nct = ns;
    } else {
      nct = x->size[1];
    }

    if (nct >= nrt) {
      m = nct;
    } else {
      m = nrt;
    }

    for (q = 0; q + 1 <= m; q++) {
      iter = (q + n * q) + 1;
      nmq = n - q;
      apply_transform = false;
      if (q + 1 <= nct) {
        ztest0 = xnrm2(nmq, A, iter);
        if (ztest0 > 0.0) {
          apply_transform = true;
          if (A->data[iter - 1] < 0.0) {
            ztest0 = -ztest0;
          }

          s->data[q] = ztest0;
          if (std::abs(s->data[q]) >= 1.0020841800044864E-292) {
            xscal(nmq, 1.0 / s->data[q], A, iter);
          } else {
            ns = iter + nmq;
            for (qs = iter; qs < ns; qs++) {
              A->data[qs - 1] /= s->data[q];
            }
          }

          A->data[iter - 1]++;
          s->data[q] = -s->data[q];
        } else {
          s->data[q] = 0.0;
        }
      }

      for (jj = q + 1; jj + 1 <= p; jj++) {
        ns = q + n * jj;
        if (apply_transform) {
          ztest0 = xdotc(nmq, A, iter, A, ns + 1);
          ztest0 = -(ztest0 / A->data[q + A->size[0] * q]);
          xaxpy(nmq, ztest0, iter, A, ns + 1);
        }

        e->data[jj] = A->data[ns];
      }

      if (q + 1 <= nct) {
        for (ns = q; ns + 1 <= n; ns++) {
          U->data[ns + U->size[0] * q] = A->data[ns + A->size[0] * q];
        }
      }

      if (q + 1 <= nrt) {
        qs = (p - q) - 1;
        ztest0 = b_xnrm2(qs, e, q + 2);
        if (ztest0 == 0.0) {
          e->data[q] = 0.0;
        } else {
          if (e->data[q + 1] < 0.0) {
            ztest0 = -ztest0;
          }

          e->data[q] = ztest0;
          ztest0 = e->data[q];
          if (std::abs(ztest0) >= 1.0020841800044864E-292) {
            ztest0 = 1.0 / ztest0;
            ns = q + qs;
            for (qs = q + 1; qs + 1 <= ns + 1; qs++) {
              e->data[qs] *= ztest0;
            }
          } else {
            ns = q + qs;
            for (qs = q + 1; qs + 1 <= ns + 1; qs++) {
              e->data[qs] /= ztest0;
            }
          }

          e->data[q + 1]++;
          e->data[q] = -e->data[q];
          if (q + 2 <= n) {
            for (ns = q + 1; ns + 1 <= n; ns++) {
              work->data[ns] = 0.0;
            }

            for (jj = q + 1; jj + 1 <= p; jj++) {
              b_xaxpy(nmq - 1, e->data[jj], A, (q + n * jj) + 2, work, q + 2);
            }

            for (jj = q + 1; jj + 1 <= p; jj++) {
              c_xaxpy(nmq - 1, -e->data[jj] / e->data[q + 1], work, q + 2, A,
                      (q + n * jj) + 2);
            }
          }
        }

        for (ns = q + 1; ns + 1 <= p; ns++) {
          Vf->data[ns + Vf->size[0] * q] = e->data[ns];
        }
      }
    }

    if (x->size[1] <= x->size[0] + 1) {
      m = x->size[1];
    } else {
      m = x->size[0] + 1;
    }

    if (nct < x->size[1]) {
      s->data[nct] = A->data[nct + A->size[0] * nct];
    }

    if (x->size[0] < m) {
      s->data[m - 1] = 0.0;
    }

    if (nrt + 1 < m) {
      e->data[nrt] = A->data[nrt + A->size[0] * (m - 1)];
    }

    e->data[m - 1] = 0.0;
    if (nct + 1 <= minnp) {
      for (jj = nct; jj + 1 <= minnp; jj++) {
        for (ns = 1; ns <= n; ns++) {
          U->data[(ns + U->size[0] * jj) - 1] = 0.0;
        }

        U->data[jj + U->size[0] * jj] = 1.0;
      }
    }

    for (q = nct - 1; q + 1 > 0; q--) {
      nmq = n - q;
      iter = q + n * q;
      if (s->data[q] != 0.0) {
        for (jj = q; jj + 2 <= minnp; jj++) {
          ns = (q + n * (jj + 1)) + 1;
          ztest0 = xdotc(nmq, U, iter + 1, U, ns);
          ztest0 = -(ztest0 / U->data[iter]);
          xaxpy(nmq, ztest0, iter + 1, U, ns);
        }

        for (ns = q; ns + 1 <= n; ns++) {
          U->data[ns + U->size[0] * q] = -U->data[ns + U->size[0] * q];
        }

        U->data[iter]++;
        for (ns = 1; ns <= q; ns++) {
          U->data[(ns + U->size[0] * q) - 1] = 0.0;
        }
      } else {
        for (ns = 1; ns <= n; ns++) {
          U->data[(ns + U->size[0] * q) - 1] = 0.0;
        }

        U->data[iter] = 1.0;
      }
    }

    for (q = x->size[1] - 1; q + 1 > 0; q--) {
      if ((q + 1 <= nrt) && (e->data[q] != 0.0)) {
        qs = (p - q) - 1;
        ns = (q + p * q) + 2;
        for (jj = q; jj + 2 <= p; jj++) {
          iter = (q + p * (jj + 1)) + 2;
          ztest0 = xdotc(qs, Vf, ns, Vf, iter);
          ztest0 = -(ztest0 / Vf->data[ns - 1]);
          xaxpy(qs, ztest0, ns, Vf, iter);
        }
      }

      for (ns = 1; ns <= p; ns++) {
        Vf->data[(ns + Vf->size[0] * q) - 1] = 0.0;
      }

      Vf->data[q + Vf->size[0] * q] = 1.0;
    }

    for (q = 0; q + 1 <= m; q++) {
      if (s->data[q] != 0.0) {
        ztest = std::abs(s->data[q]);
        ztest0 = s->data[q] / ztest;
        s->data[q] = ztest;
        if (q + 1 < m) {
          e->data[q] /= ztest0;
        }

        if (q + 1 <= n) {
          xscal(n, ztest0, U, 1 + n * q);
        }
      }

      if ((q + 1 < m) && (e->data[q] != 0.0)) {
        ztest = std::abs(e->data[q]);
        ztest0 = ztest / e->data[q];
        e->data[q] = ztest;
        s->data[q + 1] *= ztest0;
        xscal(p, ztest0, Vf, 1 + p * (q + 1));
      }
    }

    nct = m;
    iter = 0;
    snorm = 0.0;
    for (ns = 0; ns + 1 <= m; ns++) {
      ztest0 = std::abs(s->data[ns]);
      ztest = std::abs(e->data[ns]);
      if ((ztest0 >= ztest) || rtIsNaN(ztest)) {
      } else {
        ztest0 = ztest;
      }

      if (!((snorm >= ztest0) || rtIsNaN(ztest0))) {
        snorm = ztest0;
      }
    }

    while ((m > 0) && (!(iter >= 75))) {
      q = m - 1;
      exitg3 = false;
      while (!(exitg3 || (q == 0))) {
        ztest0 = std::abs(e->data[q - 1]);
        if ((ztest0 <= 2.2204460492503131E-16 *
                           (std::abs(s->data[q - 1]) + std::abs(s->data[q]))) ||
            (ztest0 <= 1.0020841800044864E-292) ||
            ((iter > 20) && (ztest0 <= 2.2204460492503131E-16 * snorm))) {
          e->data[q - 1] = 0.0;
          exitg3 = true;
        } else {
          q--;
        }
      }

      if (q == m - 1) {
        ns = 4;
      } else {
        qs = m;
        ns = m;
        exitg2 = false;
        while ((!exitg2) && (ns >= q)) {
          qs = ns;
          if (ns == q) {
            exitg2 = true;
          } else {
            ztest0 = 0.0;
            if (ns < m) {
              ztest0 = std::abs(e->data[ns - 1]);
            }

            if (ns > q + 1) {
              ztest0 += std::abs(e->data[ns - 2]);
            }

            ztest = std::abs(s->data[ns - 1]);
            if ((ztest <= 2.2204460492503131E-16 * ztest0) ||
                (ztest <= 1.0020841800044864E-292)) {
              s->data[ns - 1] = 0.0;
              exitg2 = true;
            } else {
              ns--;
            }
          }
        }

        if (qs == q) {
          ns = 3;
        } else if (qs == m) {
          ns = 1;
        } else {
          ns = 2;
          q = qs;
        }
      }

      switch (ns) {
      case 1:
        f = e->data[m - 2];
        e->data[m - 2] = 0.0;
        for (qs = m - 3; qs + 2 >= q + 1; qs--) {
          xrotg(&s->data[qs + 1], &f, &ztest0, &ztest);
          if (qs + 2 > q + 1) {
            f = -ztest * e->data[qs];
            e->data[qs] *= ztest0;
          }

          xrot(p, Vf, 1 + p * (qs + 1), 1 + p * (m - 1), ztest0, ztest);
        }
        break;

      case 2:
        f = e->data[q - 1];
        e->data[q - 1] = 0.0;
        for (qs = q; qs + 1 <= m; qs++) {
          xrotg(&s->data[qs], &f, &ztest0, &ztest);
          f = -ztest * e->data[qs];
          e->data[qs] *= ztest0;
          xrot(n, U, 1 + n * qs, 1 + n * (q - 1), ztest0, ztest);
        }
        break;

      case 3:
        varargin_1[0] = std::abs(s->data[m - 1]);
        varargin_1[1] = std::abs(s->data[m - 2]);
        varargin_1[2] = std::abs(e->data[m - 2]);
        varargin_1[3] = std::abs(s->data[q]);
        varargin_1[4] = std::abs(e->data[q]);
        ns = 1;
        mtmp = varargin_1[0];
        if (rtIsNaN(varargin_1[0])) {
          qs = 2;
          exitg1 = false;
          while ((!exitg1) && (qs < 6)) {
            ns = qs;
            if (!rtIsNaN(varargin_1[qs - 1])) {
              mtmp = varargin_1[qs - 1];
              exitg1 = true;
            } else {
              qs++;
            }
          }
        }

        if (ns < 5) {
          while (ns + 1 < 6) {
            if (varargin_1[ns] > mtmp) {
              mtmp = varargin_1[ns];
            }

            ns++;
          }
        }

        f = s->data[m - 1] / mtmp;
        ztest0 = s->data[m - 2] / mtmp;
        ztest = e->data[m - 2] / mtmp;
        sqds = s->data[q] / mtmp;
        b = ((ztest0 + f) * (ztest0 - f) + ztest * ztest) / 2.0;
        ztest0 = f * ztest;
        ztest0 *= ztest0;
        if ((b != 0.0) || (ztest0 != 0.0)) {
          ztest = std::sqrt(b * b + ztest0);
          if (b < 0.0) {
            ztest = -ztest;
          }

          ztest = ztest0 / (b + ztest);
        } else {
          ztest = 0.0;
        }

        f = (sqds + f) * (sqds - f) + ztest;
        b = sqds * (e->data[q] / mtmp);
        for (qs = q + 1; qs < m; qs++) {
          xrotg(&f, &b, &ztest0, &ztest);
          if (qs > q + 1) {
            e->data[qs - 2] = f;
          }

          f = ztest0 * s->data[qs - 1] + ztest * e->data[qs - 1];
          e->data[qs - 1] = ztest0 * e->data[qs - 1] - ztest * s->data[qs - 1];
          b = ztest * s->data[qs];
          s->data[qs] *= ztest0;
          xrot(p, Vf, 1 + p * (qs - 1), 1 + p * qs, ztest0, ztest);
          s->data[qs - 1] = f;
          xrotg(&s->data[qs - 1], &b, &ztest0, &ztest);
          f = ztest0 * e->data[qs - 1] + ztest * s->data[qs];
          s->data[qs] = -ztest * e->data[qs - 1] + ztest0 * s->data[qs];
          b = ztest * e->data[qs];
          e->data[qs] *= ztest0;
          if (qs < n) {
            xrot(n, U, 1 + n * (qs - 1), 1 + n * qs, ztest0, ztest);
          }
        }

        e->data[m - 2] = f;
        iter++;
        break;

      default:
        if (s->data[q] < 0.0) {
          s->data[q] = -s->data[q];
          xscal(p, -1.0, Vf, 1 + p * q);
        }

        ns = q + 1;
        while ((q + 1 < nct) && (s->data[q] < s->data[ns])) {
          ztest = s->data[q];
          s->data[q] = s->data[ns];
          s->data[ns] = ztest;
          if (q + 1 < p) {
            xswap(p, Vf, 1 + p * q, 1 + p * (q + 1));
          }

          if (q + 1 < n) {
            xswap(n, U, 1 + n * q, 1 + n * (q + 1));
          }

          q = ns;
          ns++;
        }

        iter = 0;
        m--;
        break;
      }
    }
  }

  emxFree_real_T(&work);
  m = e->size[0];
  e->size[0] = minnp;
  emxEnsureCapacity((emxArray__common *)e, m, (int)sizeof(double));
  for (qs = 0; qs + 1 <= minnp; qs++) {
    e->data[qs] = s->data[qs];
  }

  emxFree_real_T(&s);
  ns = x->size[1];
  m = A->size[0] * A->size[1];
  A->size[0] = ns;
  A->size[1] = minnp;
  emxEnsureCapacity((emxArray__common *)A, m, (int)sizeof(double));
  for (qs = 0; qs + 1 <= minnp; qs++) {
    for (iter = 0; iter + 1 <= p; iter++) {
      A->data[iter + A->size[0] * qs] = Vf->data[iter + Vf->size[0] * qs];
    }
  }

  m = Vf->size[0] * Vf->size[1];
  Vf->size[0] = U->size[0];
  Vf->size[1] = U->size[1];
  emxEnsureCapacity((emxArray__common *)Vf, m, (int)sizeof(double));
  qs = U->size[0] * U->size[1];
  for (m = 0; m < qs; m++) {
    Vf->data[m] = U->data[m];
  }

  emxInit_real_T(&coeff, 2);
  m = coeff->size[0] * coeff->size[1];
  coeff->size[0] = A->size[0];
  coeff->size[1] = A->size[1];
  emxEnsureCapacity((emxArray__common *)coeff, m, (int)sizeof(double));
  qs = A->size[0] * A->size[1];
  for (m = 0; m < qs; m++) {
    coeff->data[m] = A->data[m];
  }

  ns = U->size[1];
  for (qs = 0; qs + 1 <= ns; qs++) {
    for (iter = 0; iter + 1 <= nrows; iter++) {
      Vf->data[iter + Vf->size[0] * qs] *= e->data[qs];
    }
  }

  emxFree_real_T(&A);
  for (qs = 0; qs + 1 <= ns; qs++) {
    e->data[qs] = e->data[qs] * e->data[qs] / (double)DOF;
  }

  localTSquared(Vf, e, DOF, ncols, tsquared);
  if (DOF < ncols) {
    ns = U->size[1];
    if (DOF <= ns) {
      ns = DOF;
    }

    m = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = nrows;
    scoreOut->size[1] = ns;
    emxEnsureCapacity((emxArray__common *)scoreOut, m, (int)sizeof(double));
    qs = nrows * ns;
    for (m = 0; m < qs; m++) {
      scoreOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      for (iter = 0; iter + 1 <= nrows; iter++) {
        scoreOut->data[iter + scoreOut->size[0] * qs] =
            Vf->data[iter + Vf->size[0] * qs];
      }
    }

    m = latentOut->size[0];
    latentOut->size[0] = ns;
    emxEnsureCapacity((emxArray__common *)latentOut, m, (int)sizeof(double));
    for (m = 0; m < ns; m++) {
      latentOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      latentOut->data[qs] = e->data[qs];
    }

    m = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = ncols;
    coeffOut->size[1] = ns;
    emxEnsureCapacity((emxArray__common *)coeffOut, m, (int)sizeof(double));
    qs = ncols * ns;
    for (m = 0; m < qs; m++) {
      coeffOut->data[m] = 0.0;
    }

    for (qs = 0; qs + 1 <= ns; qs++) {
      for (iter = 0; iter + 1 <= ncols; iter++) {
        coeffOut->data[iter + coeffOut->size[0] * qs] =
            coeff->data[iter + coeff->size[0] * qs];
      }
    }
  } else {
    m = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = Vf->size[0];
    scoreOut->size[1] = Vf->size[1];
    emxEnsureCapacity((emxArray__common *)scoreOut, m, (int)sizeof(double));
    qs = Vf->size[0] * Vf->size[1];
    for (m = 0; m < qs; m++) {
      scoreOut->data[m] = Vf->data[m];
    }

    m = latentOut->size[0];
    latentOut->size[0] = e->size[0];
    emxEnsureCapacity((emxArray__common *)latentOut, m, (int)sizeof(double));
    qs = e->size[0];
    for (m = 0; m < qs; m++) {
      latentOut->data[m] = e->data[m];
    }

    m = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = coeff->size[0];
    coeffOut->size[1] = coeff->size[1];
    emxEnsureCapacity((emxArray__common *)coeffOut, m, (int)sizeof(double));
    qs = coeff->size[0] * coeff->size[1];
    for (m = 0; m < qs; m++) {
      coeffOut->data[m] = coeff->data[m];
    }
  }

  emxFree_real_T(&Vf);
  emxFree_real_T(&e);
  emxFree_real_T(&U);
  emxFree_real_T(&coeff);
  ztest0 = d_sum(latentOut);
  m = explained->size[0];
  explained->size[0] = latentOut->size[0];
  emxEnsureCapacity((emxArray__common *)explained, m, (int)sizeof(double));
  qs = latentOut->size[0];
  for (m = 0; m < qs; m++) {
    explained->data[m] = 100.0 * latentOut->data[m] / ztest0;
  }
}

//
// Arguments    : const emxArray_real_T *score
//                const emxArray_real_T *latent
//                int DOF
//                int p
//                emxArray_real_T *tsquared
// Return Type  : void
//
static void localTSquared(const emxArray_real_T *score,
                          const emxArray_real_T *latent, int DOF, int p,
                          emxArray_real_T *tsquared) {
  int i;
  int q;
  double absxk;
  emxArray_real_T *standScores;
  int exponent;
  if ((score->size[0] == 0) || (score->size[1] == 0)) {
    i = tsquared->size[0] * tsquared->size[1];
    tsquared->size[0] = score->size[0];
    tsquared->size[1] = score->size[1];
    emxEnsureCapacity((emxArray__common *)tsquared, i, (int)sizeof(double));
    exponent = score->size[0] * score->size[1];
    for (i = 0; i < exponent; i++) {
      tsquared->data[i] = score->data[i];
    }
  } else {
    if (DOF > 1) {
      absxk = std::abs(latent->data[0]);
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

      if (DOF >= p) {
        exponent = DOF;
      } else {
        exponent = p;
      }

      absxk *= (double)exponent;
      q = 0;
      for (exponent = 0; exponent < latent->size[0]; exponent++) {
        if (latent->data[exponent] > absxk) {
          q++;
        }
      }
    } else {
      q = 0;
    }

    emxInit_real_T(&standScores, 2);
    i = standScores->size[0] * standScores->size[1];
    standScores->size[0] = score->size[0];
    standScores->size[1] = q;
    emxEnsureCapacity((emxArray__common *)standScores, i, (int)sizeof(double));
    exponent = score->size[0];
    i = tsquared->size[0] * tsquared->size[1];
    tsquared->size[0] = exponent;
    tsquared->size[1] = 1;
    emxEnsureCapacity((emxArray__common *)tsquared, i, (int)sizeof(double));
    exponent = score->size[0];
    for (i = 0; i < exponent; i++) {
      tsquared->data[i] = 0.0;
    }

    for (exponent = 0; exponent + 1 <= q; exponent++) {
      absxk = std::sqrt(latent->data[exponent]);
      for (i = 0; i + 1 <= score->size[0]; i++) {
        standScores->data[i + standScores->size[0] * exponent] =
            score->data[i + score->size[0] * exponent] / absxk;
        tsquared->data[i] +=
            standScores->data[i + standScores->size[0] * exponent] *
            standScores->data[i + standScores->size[0] * exponent];
      }
    }

    emxFree_real_T(&standScores);
  }
}

//
// Arguments    : emxArray_real_T *x
//                int NumComponents
//                const emxArray_real_T *Weights
//                const emxArray_real_T *VariableWeights
//                emxArray_real_T *coeffOut
//                emxArray_real_T *scoreOut
//                emxArray_real_T *latent
// Return Type  : void
//
static void local_pca(emxArray_real_T *x, int NumComponents,
                      const emxArray_real_T *Weights,
                      const emxArray_real_T *VariableWeights,
                      emxArray_real_T *coeffOut, emxArray_real_T *scoreOut,
                      emxArray_real_T *latent) {
  emxArray_int32_T *naninfo_nNaNsInRow;
  int n;
  int p;
  int b_n;
  int m;
  int naninfo_nRowsWithNaNs;
  int j;
  int nc2;
  emxArray_boolean_T *naninfo_isNaN;
  int i;
  boolean_T noNaNs;
  int DOF;
  unsigned int sz[2];
  emxArray_real_T *mu;
  double wcol;
  double xcol;
  emxArray_real_T *coeff;
  emxArray_real_T *score;
  emxArray_real_T *tsquared;
  emxArray_real_T *explained;
  emxArray_real_T *WNoNaNs;
  emxArray_real_T *b_x;
  double absc;
  emxInit_int32_T(&naninfo_nNaNsInRow, 1);
  n = x->size[0];
  p = x->size[1];
  b_n = x->size[0];
  m = 0;
  naninfo_nRowsWithNaNs = 0;
  j = naninfo_nNaNsInRow->size[0];
  naninfo_nNaNsInRow->size[0] = x->size[0];
  emxEnsureCapacity((emxArray__common *)naninfo_nNaNsInRow, j,
                    (int)sizeof(int));
  nc2 = x->size[0];
  for (j = 0; j < nc2; j++) {
    naninfo_nNaNsInRow->data[j] = 0;
  }

  emxInit_boolean_T1(&naninfo_isNaN, 2);
  j = naninfo_isNaN->size[0] * naninfo_isNaN->size[1];
  naninfo_isNaN->size[0] = x->size[0];
  naninfo_isNaN->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)naninfo_isNaN, j,
                    (int)sizeof(boolean_T));
  nc2 = x->size[0] * x->size[1];
  for (j = 0; j < nc2; j++) {
    naninfo_isNaN->data[j] = rtIsNaN(x->data[j]);
  }

  for (j = 1; j <= x->size[1]; j++) {
    for (i = 0; i + 1 <= b_n; i++) {
      if (naninfo_isNaN->data[i + naninfo_isNaN->size[0] * (j - 1)]) {
        naninfo_nNaNsInRow->data[i]++;
        m++;
      }
    }
  }

  emxFree_boolean_T(&naninfo_isNaN);
  for (i = 1; i <= b_n; i++) {
    if (naninfo_nNaNsInRow->data[i - 1] > 0) {
      naninfo_nRowsWithNaNs++;
    }
  }

  noNaNs = !(m > 0);
  DOF = x->size[0] - naninfo_nRowsWithNaNs;
  if (DOF >= 1) {
    DOF--;
  }

  m = x->size[0];
  b_n = x->size[1];
  for (j = 0; j < 2; j++) {
    sz[j] = (unsigned int)x->size[j];
  }

  emxInit_real_T(&mu, 2);
  j = mu->size[0] * mu->size[1];
  mu->size[0] = 1;
  mu->size[1] = (int)sz[1];
  emxEnsureCapacity((emxArray__common *)mu, j, (int)sizeof(double));
  nc2 = (int)sz[1];
  for (j = 0; j < nc2; j++) {
    mu->data[j] = 0.0;
  }

  if (!noNaNs) {
    for (j = 0; j + 1 <= b_n; j++) {
      wcol = 0.0;
      xcol = 0.0;
      for (i = 0; i + 1 <= m; i++) {
        if (!rtIsNaN(x->data[i + x->size[0] * j])) {
          wcol++;
          xcol += x->data[i + x->size[0] * j];
        }
      }

      mu->data[j] = xcol / wcol;
    }
  } else {
    for (j = 0; j + 1 <= b_n; j++) {
      wcol = 0.0;
      xcol = 0.0;
      for (i = 1; i <= m; i++) {
        wcol++;
        xcol += x->data[(i + x->size[0] * j) - 1];
      }

      mu->data[j] = xcol / wcol;
    }
  }

  for (j = 0; j + 1 <= p; j++) {
    for (i = 0; i + 1 <= n; i++) {
      x->data[i + x->size[0] * j] -= mu->data[j];
    }
  }

  emxFree_real_T(&mu);
  emxInit_real_T(&coeff, 2);
  emxInit_real_T(&score, 2);
  emxInit_real_T(&tsquared, 2);
  emxInit_real_T1(&explained, 1);
  if (noNaNs) {
    localSVD(x, DOF, Weights, VariableWeights, coeff, score, latent, tsquared,
             explained);
  } else {
    emxInit_real_T1(&WNoNaNs, 1);
    m = x->size[0] - naninfo_nRowsWithNaNs;
    j = score->size[0] * score->size[1];
    score->size[0] = m;
    score->size[1] = x->size[1];
    emxEnsureCapacity((emxArray__common *)score, j, (int)sizeof(double));
    j = WNoNaNs->size[0];
    WNoNaNs->size[0] = m;
    emxEnsureCapacity((emxArray__common *)WNoNaNs, j, (int)sizeof(double));
    m = -1;
    for (i = 0; i + 1 <= x->size[0]; i++) {
      if (naninfo_nNaNsInRow->data[i] == 0) {
        m++;
        for (j = 0; j + 1 <= x->size[1]; j++) {
          score->data[m + score->size[0] * j] = x->data[i + x->size[0] * j];
        }

        WNoNaNs->data[m] = 1.0;
      }
    }

    emxInit_real_T(&b_x, 2);
    b_localSVD(score, DOF, WNoNaNs, VariableWeights, coeff, b_x, latent,
               tsquared, explained);
    p = b_x->size[1];
    j = score->size[0] * score->size[1];
    score->size[0] = naninfo_nNaNsInRow->size[0];
    score->size[1] = b_x->size[1];
    emxEnsureCapacity((emxArray__common *)score, j, (int)sizeof(double));
    m = -1;
    i = 0;
    emxFree_real_T(&WNoNaNs);
    while (i + 1 <= naninfo_nNaNsInRow->size[0]) {
      if (naninfo_nNaNsInRow->data[i] > 0) {
        for (j = 1; j <= p; j++) {
          score->data[i + score->size[0] * (j - 1)] = rtNaN;
        }
      } else {
        m++;
        for (j = 0; j + 1 <= p; j++) {
          score->data[i + score->size[0] * j] = b_x->data[m + b_x->size[0] * j];
        }
      }

      i++;
    }

    emxFree_real_T(&b_x);
  }

  emxFree_real_T(&explained);
  emxFree_real_T(&tsquared);
  emxFree_int32_T(&naninfo_nNaNsInRow);
  n = score->size[0];
  if (NumComponents < DOF) {
    j = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = coeff->size[0];
    coeffOut->size[1] = NumComponents;
    emxEnsureCapacity((emxArray__common *)coeffOut, j, (int)sizeof(double));
    for (j = 0; j + 1 <= NumComponents; j++) {
      for (i = 0; i + 1 <= coeff->size[0]; i++) {
        coeffOut->data[i + coeffOut->size[0] * j] =
            coeff->data[i + coeff->size[0] * j];
      }
    }

    j = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = score->size[0];
    scoreOut->size[1] = NumComponents;
    emxEnsureCapacity((emxArray__common *)scoreOut, j, (int)sizeof(double));
    for (j = 0; j + 1 <= NumComponents; j++) {
      for (i = 0; i + 1 <= n; i++) {
        scoreOut->data[i + scoreOut->size[0] * j] =
            score->data[i + score->size[0] * j];
      }
    }
  } else {
    j = coeffOut->size[0] * coeffOut->size[1];
    coeffOut->size[0] = coeff->size[0];
    coeffOut->size[1] = coeff->size[1];
    emxEnsureCapacity((emxArray__common *)coeffOut, j, (int)sizeof(double));
    nc2 = coeff->size[0] * coeff->size[1];
    for (j = 0; j < nc2; j++) {
      coeffOut->data[j] = coeff->data[j];
    }

    j = scoreOut->size[0] * scoreOut->size[1];
    scoreOut->size[0] = score->size[0];
    scoreOut->size[1] = score->size[1];
    emxEnsureCapacity((emxArray__common *)scoreOut, j, (int)sizeof(double));
    nc2 = score->size[0] * score->size[1];
    for (j = 0; j < nc2; j++) {
      scoreOut->data[j] = score->data[j];
    }
  }

  emxFree_real_T(&score);
  emxFree_real_T(&coeff);
  m = coeffOut->size[0];
  nc2 = coeffOut->size[1];
  for (j = 0; j + 1 <= nc2; j++) {
    wcol = 0.0;
    xcol = 1.0;
    for (i = 0; i + 1 <= m; i++) {
      absc = std::abs(coeffOut->data[i + coeffOut->size[0] * j]);
      if (absc > wcol) {
        wcol = absc;
        if (coeffOut->data[i + coeffOut->size[0] * j] < 0.0) {
          xcol = -1.0;
        } else if (coeffOut->data[i + coeffOut->size[0] * j] > 0.0) {
          xcol = 1.0;
        } else if (coeffOut->data[i + coeffOut->size[0] * j] == 0.0) {
          xcol = 0.0;
        } else {
          xcol = coeffOut->data[i + coeffOut->size[0] * j];
        }
      }
    }

    if (xcol < 0.0) {
      for (i = 0; i + 1 <= m; i++) {
        coeffOut->data[i + coeffOut->size[0] * j] =
            -coeffOut->data[i + coeffOut->size[0] * j];
      }

      for (i = 0; i + 1 <= n; i++) {
        scoreOut->data[i + scoreOut->size[0] * j] =
            -scoreOut->data[i + scoreOut->size[0] * j];
      }
    }
  }
}

//
// Arguments    : const emxArray_real_T *x
//                emxArray_real_T *varargout_1
//                emxArray_real_T *varargout_2
//                emxArray_real_T *varargout_3
// Return Type  : void
//
void pca(const emxArray_real_T *x, emxArray_real_T *varargout_1,
         emxArray_real_T *varargout_2, emxArray_real_T *varargout_3) {
  emxArray_real_T *b_x;
  int i2;
  int loop_ub;
  emxArray_real_T *r6;
  emxArray_real_T *r7;
  emxInit_real_T(&b_x, 2);
  i2 = b_x->size[0] * b_x->size[1];
  b_x->size[0] = x->size[0];
  b_x->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)b_x, i2, (int)sizeof(double));
  loop_ub = x->size[0] * x->size[1];
  for (i2 = 0; i2 < loop_ub; i2++) {
    b_x->data[i2] = x->data[i2];
  }

  emxInit_real_T(&r6, 2);
  i2 = r6->size[0] * r6->size[1];
  r6->size[0] = 1;
  r6->size[1] = x->size[0];
  emxEnsureCapacity((emxArray__common *)r6, i2, (int)sizeof(double));
  loop_ub = x->size[0];
  for (i2 = 0; i2 < loop_ub; i2++) {
    r6->data[i2] = 1.0;
  }

  emxInit_real_T(&r7, 2);
  i2 = r7->size[0] * r7->size[1];
  r7->size[0] = 1;
  r7->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)r7, i2, (int)sizeof(double));
  loop_ub = x->size[1];
  for (i2 = 0; i2 < loop_ub; i2++) {
    r7->data[i2] = 1.0;
  }

  local_pca(b_x, x->size[1], r6, r7, varargout_1, varargout_2, varargout_3);
  emxFree_real_T(&r7);
  emxFree_real_T(&r6);
  emxFree_real_T(&b_x);
}
