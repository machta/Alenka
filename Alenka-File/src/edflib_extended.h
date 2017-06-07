#ifndef EDFLIB_EXTENDED_H
#define EDFLIB_EXTENDED_H

#include <edflib.h>

void edf_set_gender_char(int handle, const char *gender);
void edf_set_birthdate_char(int handle, const char *birthdate);

// This was my attempt at solving a problem with the library's interface.
// Unfortunately edf_set_birthdate_char causes, for some reason, the resulting
// file to be in an inconsistent format.

#endif // EDFLIB_EXTENDED_H
