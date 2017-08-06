
// Include Files
#include "fileManager.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : FILE * *f
//                boolean_T *a
// Return Type  : void
//
void fileManager(FILE **f, boolean_T *a) {
  *f = stderr;
  *a = true;
}
