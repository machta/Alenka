#ifndef SPIKE_CLUSTER_CPP_H
#define SPIKE_CLUSTER_CPP_H

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
extern int getThreadID();
extern void getThreadID_init();
extern void spike_cluster_cpp(const emxArray_real_T *MA,
                              const emxArray_real_T *MW, double pca_centering,
                              struct0_T *cluster);

#endif
