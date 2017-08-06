#ifndef SORTIDX_H
#define SORTIDX_H

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
extern void b_sortIdx(emxArray_real_T *x, emxArray_int32_T *idx);
extern void c_sortIdx(double x_data[], int x_size[1], int idx_data[],
                      int idx_size[1]);
extern void sortIdx(const emxArray_real_T *x, emxArray_int32_T *idx);

#endif
