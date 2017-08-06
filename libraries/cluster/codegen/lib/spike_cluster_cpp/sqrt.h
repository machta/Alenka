#ifndef SQRT_H
#define SQRT_H

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
extern void b_sqrt(emxArray_real_T *x);
extern void c_sqrt(emxArray_real_T *x);

#endif
