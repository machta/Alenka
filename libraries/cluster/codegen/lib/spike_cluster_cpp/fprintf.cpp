
// Include Files
#include "fprintf.h"
#include "fileManager.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : void
// Return Type  : int
//
int cfprintf() {
  int nbytesint;
  FILE *b_NULL;
  FILE *filestar;
  boolean_T autoflush;
  static const char cfmt[24] = {'W', 'a', 'r', 'n', 'i', 'n', 'g',    ':',
                                ' ', 's', 'i', 'z', 'e', ' ', 'm',    'i',
                                's', 'm', 'a', 't', 'c', 'h', '\x0a', '\x00'};

  b_NULL = NULL;
  nbytesint = 0;
  fileManager(&filestar, &autoflush);
  if (!(filestar == b_NULL)) {
    nbytesint = fprintf(filestar, "%s", cfmt);
    fflush(filestar);
  }

  return nbytesint;
}
