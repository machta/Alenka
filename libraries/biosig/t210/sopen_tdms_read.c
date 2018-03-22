/*

    Copyright (C) 2010,2011,2012,2015,2016 Alois Schloegl <alois.schloegl@ist.ac.at>

    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/

    BioSig is free software; you can redistribute it and/or
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
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../biosig-dev.h"

EXTERN_C void sopen_tdms_read(HDRTYPE* hdr) {
/*
	this function will be called by the function SOPEN in "biosig.c"

	Input:
		char* Header	// contains the file content

	Output:
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/

if (VERBOSE_LEVEL>7) fprintf(stdout,"%s:%i %s started - %i bytes already loaded\n",__FILE__,__LINE__,__func__,hdr->HeadLen);

		/*
			Specification obtained from http://www.ni.com/white-paper/5696/en
			however, there are also TDMS data out there, that is based on an XML header
			and a separate binary file. This is somewhat confusing.
		 */
		fprintf(stderr,"%s (line %i): Format TDMS is very experimental\n",__func__,__LINE__);

#define kTocMetaData 		(1L<<1)
#define kTocRawData 		(1L<<3)
#define kTocDAQmxRawData 	(1L<<7)
#define kTocInterleavedData 	(1L<<5)
#define kTocBigEndian 		(1L<<6)
#define kTocNewObjList 		(1L<<2)

		/***** Lead In *****/
		hdr->FILE.LittleEndian     = (leu32p(hdr->AS.Header+4) & kTocBigEndian) != 0; 	// affects only raw data
		hdr->VERSION 	           = leu32p(hdr->AS.Header+8);
		uint64_t nextSegmentOffset = leu64p(hdr->AS.Header+12);
		uint64_t RawDataOffset     = leu64p(hdr->AS.Header+20);
		size_t pos = 28;

		/***** Meta data *****/
		while (pos<RawDataOffset) {
			uint32_t numberOfObjects  = leu32p(hdr->AS.Header+pos);
			pos += 4;
			char *pstr=NULL;
			for (uint32_t k=0; k < numberOfObjects; k++) {
				uint32_t plen = leu32p(hdr->AS.Header+pos);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %i): %i %i %i\n",__func__,__LINE__, (int)hdr->HeadLen, (int)pos,plen);
				pstr  = realloc(pstr,plen+1);
				memcpy(pstr,hdr->AS.Header+pos+4,plen);
				pstr[plen]=0;

				pos += 4+plen;
				uint32_t idx  = leu32p(hdr->AS.Header+pos);
				pos += 4;
				uint32_t numberOfProperties  = leu32p(hdr->AS.Header+pos);
				pos += 4;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %i): Object %i/%i <%s> has %i properties \n",
				__func__,__LINE__, k,numberOfObjects,pstr,numberOfProperties);

				char *propName=NULL;
				char *propVal=NULL;
				for (uint32_t p=0; p < numberOfProperties; p++) {
					// property name
					uint32_t plen = leu32p(hdr->AS.Header+pos);
					propName  = realloc(propName,plen+1);
					memcpy(propName,hdr->AS.Header+pos+4,plen);
					propName[plen]=0;
					pos += 4+plen;

	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i/%i <%s>\n",__func__,__LINE__, k, p, numberOfProperties, propName);

					// property type
					uint32_t propType = leu32p(hdr->AS.Header+pos);
					pos += 4;


					// property value
					int32_t val_i32;
					switch (propType) {
					case 0x20:
						plen = leu32p(hdr->AS.Header+pos);
						propVal  = realloc(propVal,plen+1);
						memcpy(propVal,hdr->AS.Header+pos+4,plen);
						propVal[plen]=0;
						pos += 4+plen;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(string)=<%s>\n",__func__,__LINE__, k, p,propName, propVal);
						break;

					case 0x03: {
						// int32
						int32_t val = lei32p(hdr->AS.Header+pos);
						pos += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case 0x07: {
						// uint32
						uint32_t val = leu32p(hdr->AS.Header+pos);
						pos += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					default:
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>type=%i not supported\n",__func__,__LINE__, k, p,propName,propType);
					}
				}


	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i/%i path='%s' rawDataIdx=0x%08x\n",__func__,__LINE__, k, numberOfObjects, pstr, idx);

				switch (idx) {
				case 0xffffffff :	// no raw data
					break;
				case 0x00001269 :	// DAQmx Format Changing scaler
					break;
				case 0x00001369 :	// DAQmx Digital Line scaler
					break;
				case 0x00000000 : 	//
					;
				}


			}
		}

		biosigERROR(hdr,B4C_FORMAT_UNSUPPORTED,"Format TDMS is currently not supported");
}

