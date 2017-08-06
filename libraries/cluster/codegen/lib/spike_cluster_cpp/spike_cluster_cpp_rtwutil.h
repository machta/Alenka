#ifndef SPIKE_CLUSTER_CPP_RTWUTIL_H
#define SPIKE_CLUSTER_CPP_RTWUTIL_H

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
extern void emlrtFreeThreadStackData();
extern spike_cluster_cppTLS *emlrtGetThreadStackData();
extern void emlrtInitThreadStackData();
extern double rt_roundd_snf(double u);

#endif
