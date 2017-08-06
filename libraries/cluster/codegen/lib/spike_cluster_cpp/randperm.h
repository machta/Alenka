#ifndef RANDPERM_H
#define RANDPERM_H

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
extern void b_randperm(double n, double p[50000]);
extern void c_randperm(double n, emxArray_real_T *p);
extern void randperm(double n, double p[375000]);

#endif
