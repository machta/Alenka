
// Include Files
#include "unique.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *b
//                int nb
//                int *nMInf
//                int *nFinite
//                int *nPInf
//                int *nNaN
// Return Type  : void
//
void count_nonfinites(const emxArray_real_T *b, int nb, int *nMInf,
                      int *nFinite, int *nPInf, int *nNaN) {
  *nMInf = 0;
  *nFinite = nb;
  while ((*nFinite >= 1) && rtIsNaN(b->data[*nFinite - 1])) {
    (*nFinite)--;
  }

  *nNaN = nb - *nFinite;
  while ((*nFinite >= 1) && rtIsInf(b->data[*nFinite - 1]) &&
         (b->data[*nFinite - 1] > 0.0)) {
    (*nFinite)--;
  }

  *nPInf = (nb - *nFinite) - *nNaN;
}
