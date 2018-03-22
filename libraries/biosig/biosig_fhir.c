/*

% Copyright (C) 2016 Alois Schloegl <alois.schloegl@ist.ac.at>
% This file is part of the "BioSig for C/C++" repository
% (biosig4c++) at http://biosig.sf.net/


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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <b64/cencode.h>
#include "biosig-dev.h"
#include "biosig.h"

#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define max(a,b)        (((a) > (b)) ? (a) : (b))

/*
  https://www.hl7.org/fhir/binary.html
  https://catalyze.io/learn/the-fhir-resource-object-the-core-building-block
*/

/*
   biosig_hdr2gdf_base64(..)
     converts biosig data into GDF-stream encoded in base64

   hdr: biosig header structure, 
        file must be open in order to read data section
   fid: stream for data output
   
*/
int biosig_hdr2gdf_base64(HDRTYPE *hdr, FILE *fid) {

	hdr->TYPE=GDF;
	hdr->VERSION=3.0;

	struct2gdfbin(hdr);
	size_t len3 = hdrEVT2rawEVT(hdr);
	size_t len0 = hdr->HeadLen + hdr->NRec*hdr->AS.bpb + len3;
	
	size_t buflen = max(max(len3,hdr->HeadLen),hdr->AS.bpb);
	char *buf     = (char*)malloc(buflen*2);
	char *mem     = (char*)malloc(buflen);

	base64_encodestate B64STATE;
	base64_init_encodestate(&B64STATE);

	// write header	
	int c = base64_encode_block((char*)hdr->AS.Header, (int)hdr->HeadLen, buf, &B64STATE);
	fwrite(buf,1,c,fid);

	// write data block	
	size_t count = 0;
	size_t bpb = bpb8_collapsed_rawdata(hdr)>>3;
	size_t blks= buflen/hdr->AS.bpb;
	for (size_t nrec=0; nrec < hdr->NRec; nrec+=blks) {
		count = sread_raw(nrec, min(blks, hdr->NRec-nrec), hdr, 1, mem, blks*hdr->AS.bpb);
		c = base64_encode_block(mem, bpb, buf, &B64STATE);
		fwrite(buf,1,c,fid);
	}

	// write event table
	c = base64_encode_block((char*)hdr->AS.rawEventData, (int)len3, buf, &B64STATE);
	fwrite(buf,1,c,fid);

	c = base64_encode_blockend(buf, &B64STATE);
	fwrite(buf,1,c,fid);

	if (buf) free(buf);
	if (mem) free(mem);
	
	return(0);
}

/*
   biosig_fhir_binary_json_template:
      opens a biosig file, and generates binary json template according to fhir
*/

char* biosig_fhir_binary_xml_template(const char *filename, FILE *fid) {
	/* generate XML template
	/*
	<Binary xmlns="http://hl7.org/fhir">
	 <!-- from Resource: id, meta, implicitRules, and language -->
	 <contentType value="[code]"/><!-- 1..1 MimeType of the binary content  -->
	 <content value="[base64Binary]"/><!-- 1..1 The actual content -->
	</Binary>
	*/

	HDRTYPE *hdr = NULL;
	hdr = sopen(filename, "r", hdr);

	fprintf(fid,"<Binary xmlns=\"http://hl7.org/fhir\">\n"
		"  <!-- from Resource: id, meta, implicitRules, and language -->\n"
		"  <contentType value=\"X-biosig/gdf\"/>\n"
		"  <content value=\"");

	biosig_hdr2gdf_base64(hdr, fid);

	fprintf(fid,"\"/>\n"
		"</Binary>\n");

	destructHDR(hdr);
	return(0);
}


char* biosig_fhir_binary_json_template(const char *filename, FILE *fid) {
	/* generate json template
	{
	  "resourceType" : "Binary",
	  "id" : filename,	
	  "meta" : { asprintf_hdr2json(&str, filename),
	   // from Resource: id, meta, implicitRules, and language
	  "contentType" : "<code>", // R!  MimeType of the binary content 
	  "content" : "<base64Binary>" // R!  The actual content
	}
	*/
	HDRTYPE *hdr = NULL;
	hdr = sopen(filename, "r", hdr);

	fprintf(fid,"{\n"	
		  "    \"resourceType\" : \"Binary\",\n"
		  "    \"id\" : \"%s\",\n"
		  "    \"meta\" : ", hdr->FileName);

	fprintf_hdr2json(fid, hdr);

	fprintf(fid, ",    \"contentType\" : \"X-biosig/gdf\",\n"
		     "    \"content\" : \"");

	biosig_hdr2gdf_base64(hdr, fid);

	fprintf(fid, "\"\n}\n");

	destructHDR(hdr);
	return(0);
}

#ifndef NDEBUG
extern int VERBOSE_LEVEL;
#endif

#ifdef __cplusplus
}
#endif


const char *help =	"%s provides fhir binary template for biosignal data\n\n"
			"    Usage: %s [-json|-xml|-base64] <filename>\n\n"
			"       reads filename and converts it fhir-(json|xml)-binary-template\n"
			"       or a media-type X-biosig/gdf (i.e. base64-encoded GDF stream).\n"
			"   libbiosig version %06x is used to read the file.\n\n";

int main(int argc, char **argv) {

#ifndef NDEBUG
    	VERBOSE_LEVEL = 0;
#endif

	enum {BASE64,JSON,XML} flag_output_format = JSON;

	if (argc<2) {
		fprintf(stderr,help, argv[0], argv[0], get_biosig_version());
		exit;
	}

	char **opt = argv+1;

	for (; *opt!=NULL; opt++) {
		if (!strcasecmp(*opt, "-base64")) {
			flag_output_format = BASE64;
		}
		if (!strcasecmp(*opt, "-json")) {
			flag_output_format = JSON;
		}
		else if (!strcasecmp(*opt, "-xml")) {
			flag_output_format = XML;
		}
#ifndef NDEBUG
		else if (!strncmp(*opt, "-V", 2)) {
			VERBOSE_LEVEL = (*opt)[2]-'0';
		}
#endif
		else if (!strcasecmp(*opt, "-h") || !strcasecmp(*opt, "--help")) {
			fprintf(stderr,help, argv[0], argv[0], get_biosig_version());
		}
		else if (*opt[0] != '-') {
			HDRTYPE *hdr = NULL;
			const char *filename = *opt;
			switch (flag_output_format) {
			case BASE64:
				hdr = sopen(filename, "r", hdr);
				biosig_hdr2gdf_base64(hdr, stdout);
				destructHDR(hdr);
				break;
			case JSON:
				biosig_fhir_binary_json_template(filename, stdout);
				break;
			case XML:
				biosig_fhir_binary_xml_template(filename, stdout);
				break;
			}
		}
	}
}

