#ifndef NULLASSIGNMENT_H
#define NULLASSIGNMENT_H

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
extern void nullAssignment(emxArray_real_T *x, const emxArray_boolean_T *idx);

#endif
