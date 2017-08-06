#ifndef SUM_H
#define SUM_H

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
extern void b_sum(const emxArray_real_T *x, emxArray_real_T *y);
extern void c_sum(const emxArray_boolean_T *x, emxArray_real_T *y);
extern double d_sum(const emxArray_real_T *x);
extern double sum(const emxArray_boolean_T *x);

#endif
