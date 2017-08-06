#ifndef REPMAT_H
#define REPMAT_H

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
extern void repmat(const emxArray_real_T *a, double varargin_1,
                   emxArray_real_T *b);

#endif
