/*
    Copyright (C) 2016 Alois Schloegl <alois.schloegl@gmail.com>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++/libbiosig) at http://biosig.sf.net/

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <R.h>
#include <Rdefines.h>
#include <R_ext/Rdynload.h>

#include "../biosig.h"

HDRTYPE *hdr=NULL;


SEXP sload(SEXP filename, SEXP channels) {

    SEXP result = R_NilValue;

    // sanity check of input 
    if(!isString(filename) || length(filename) != 1)
        error("filename is not a single string");
        
    const char* fn = CHAR(asChar(filename));

    // Open file 
    HDRTYPE *hdr = sopen(fn, "r", NULL);
    if (serror2(hdr)) return NULL;   

    long NS = biosig_get_number_of_channels(hdr);	
  
    // allocate memory for results
    result=PROTECT(allocMatrix(REALSXP, hdr->SPR*hdr->NRec, NS));

    // read data from file and write into result
    int status = sread(REAL(result), 0, hdr->SPR*hdr->NRec, hdr); 
    if (serror2(hdr)) {
        destructHDR(hdr);
	Free(result);
	UNPROTECT(1);
        return NULL;   
    }

    // close file, and cleanup memory to avoid any leaks 
    destructHDR(hdr);  

    UNPROTECT(1);
    return result;
}

