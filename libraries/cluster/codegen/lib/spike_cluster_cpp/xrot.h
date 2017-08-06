#ifndef XROT_H
#define XROT_H

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
extern void xrot(int n, emxArray_real_T *x, int ix0, int iy0, double c,
                 double s);

#endif
