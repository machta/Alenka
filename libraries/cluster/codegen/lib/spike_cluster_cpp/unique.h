#ifndef UNIQUE_H
#define UNIQUE_H

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
extern void count_nonfinites(const emxArray_real_T *b, int nb, int *nMInf,
                             int *nFinite, int *nPInf, int *nNaN);

#endif
