#ifndef EML_RAND_H
#define EML_RAND_H

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
extern void b_eml_rand(double varargin_2, emxArray_real_T *r);
extern double eml_rand();
extern void eml_rand_init();
extern void eml_rand_swap();

#endif
