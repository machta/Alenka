#include "edflib_extended.h"

#include <edflib.c>

#include <cassert>

void edf_set_gender_char(int handle, const char *gender) {
  assert(0);
  memcpy(hdrlist[handle]->plus_gender, gender, 16);
}

void edf_set_birthdate_char(int handle, const char *birthdate) {
  assert(0);
  memcpy(hdrlist[handle]->plus_birthdate, birthdate, 10);
}
