#ifndef PCA_H
#define PCA_H

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
extern void pca(const emxArray_real_T *x, emxArray_real_T *varargout_1,
                emxArray_real_T *varargout_2, emxArray_real_T *varargout_3);

#endif
