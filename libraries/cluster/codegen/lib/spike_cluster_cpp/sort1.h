#ifndef SORT1_H
#define SORT1_H

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
extern void c_sort(double x_data[], int x_size[2]);
extern void d_sort(emxArray_real_T *x, emxArray_int32_T *idx);
extern void f_sort(emxArray_real_T *x, emxArray_int32_T *idx);
extern void sort(emxArray_real_T *x, emxArray_int32_T *idx);

#endif
