#ifndef EML_RAND_MT19937AR_STATEFUL_H
#define EML_RAND_MT19937AR_STATEFUL_H

// Include Files
#include "omp.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "spike_cluster_cpp_types.h"
#include <cmath>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Function Declarations
extern void b_eml_rand_mt19937ar_stateful(double varargin_2,
                                          emxArray_real_T *r);
extern void c_eml_rand_mt19937ar_stateful_i();
extern void c_eml_rand_mt19937ar_stateful_s(boolean_T aToMaster);
extern double eml_rand_mt19937ar_stateful();

#endif
