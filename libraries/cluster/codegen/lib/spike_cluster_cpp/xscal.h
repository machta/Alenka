#ifndef XSCAL_H
#define XSCAL_H

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
extern void xscal(int n, double a, emxArray_real_T *x, int ix0);

#endif
