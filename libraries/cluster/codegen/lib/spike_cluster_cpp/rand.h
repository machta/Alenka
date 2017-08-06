#ifndef RAND_H
#define RAND_H

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
extern double b_rand();
extern void c_rand(double varargin_2, emxArray_real_T *r);

#endif
