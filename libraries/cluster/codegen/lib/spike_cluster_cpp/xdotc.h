#ifndef XDOTC_H
#define XDOTC_H

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
extern double xdotc(int n, const emxArray_real_T *x, int ix0,
                    const emxArray_real_T *y, int iy0);

#endif
