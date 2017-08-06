#ifndef COMBINE_VECTOR_ELEMENTS_H
#define COMBINE_VECTOR_ELEMENTS_H

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
extern void combine_vector_elements(const emxArray_real_T *x,
                                    emxArray_real_T *y);

#endif
