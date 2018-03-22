/*

    Copyright (C) 2015 Alois Schloegl <alois.schloegl@ist.ac.at>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 
 

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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "physicalunits.h"

const char help[] = 
		"\npu is a program to convert physical units from string\n" 
		"\trepresentation into numerical constants, according to\n"
		"\tISO/IEEE 11073-10101:2004 Vital Signs Units of Measurement\n\n"
		"Usage:\n\tpu units\n\tpu code (decimal or hexadecimal)\n\n"
		"The output returns the <physical unit>,<decimal code>,<hexadecimal code>,<scaling>,<physical unit w/o prefix>\n\n"
		"Examples:\n   pu mV\n"
		"\tuV	4275	0x10b3	1e-06	V\n\n"
		"   pu 4275\n"
		"\tuV	4275	0x10b3	1e-06	V\n\n"
		"   pu mV nA 4180 kg dag degree \"kg l-1\" rpm\n"
		"\tmV	4274	0x10b2	0.001	V\n"
		"\tnA	4180	0x1054	1e-09	A\n"
		"\tnA	4180	0x1054	1e-09	A\n"
		"\tkg	1731	0x6c3	1000	g\n"
		"\tdag	1729	0x6c1	10	g\n"
		"\tdegree	736	0x2e0	1	degree\n"
		"\tkg l-1	2015	0x0803	1000	g l-1\n"
		"\trpm	6816	0x1aa0	1	rpm\n";

int main(int argc, char **argv) {

	uint16_t pdc;
	const char *in;
	double scale; 
	const char *out1; 
	const char *out2; 
	char *E;

    if (argc<2)
	fprintf(stdout,"%s", help);

    for (int k=1; k<argc; k++) {
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") ) {
		fprintf(stdout,"pu (BioSig4C++)\n");
		fprintf(stdout,"Copyright (C) 2015 by Alois Schloegl and others\n");
		fprintf(stdout,"This file is part of BioSig http://biosig.sf.net - the free and\n");
		fprintf(stdout,"open source software library for biomedical signal processing.\n\n");
		fprintf(stdout,"BioSig is free software; you can redistribute it and/or modify\n");
		fprintf(stdout,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stdout,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stdout,"(at your option) any later version.\n\n");
	}	
    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") ) {
		fprintf(stdout,"%s", help);
	}
	else {

		in  = argv[k];
		long L = strtol(in,&E,0);

		if (*E) {
			pdc   = PhysDimCode(in);
			scale = PhysDimScale(pdc);
			out1  = in;
			out2  = PhysDim3(pdc & 0xffe0);	
		}
		else {
			pdc = L;
			scale = PhysDimScale(pdc);
			out1   = PhysDim3(pdc);
			out2   = PhysDim3(pdc & 0xffe0);
		}
		fprintf(stdout,"%s\t%u\t0x%04x\t%g\t%s\n",out1,pdc,pdc,scale,out2);
	}
    }
}

