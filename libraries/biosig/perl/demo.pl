#!/usr/bin/perl
#
# Demonstrates how the Header information of biosig data can be read through 
#    JSON header export 
#
# References: 
# [1] https://www.hl7.org/fhir/references.html
# [2] ÖNorm K2204:2015, GDF - A general data format for biomedical signals


#
#   Copyright (C) 2016 Alois Schloegl <alois.schloegl@ist.ac.at>
#   This file is part of the BioSig repository at http://biosig.sf.net/
#
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 3
#    of the License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;

use JSON::XS;
use Data::Dumper;

my $EXT='';
if ("$^O" eq "MSWin32") {
	$EXT='.exe';
}


# biosig_fhir_base64: 
#    encodes a biosig file as a base64-encoded GDF [2] v3.0 stream
sub biosig_fhir_base64 {
	my $cmd = "biosig_fhir$EXT -base64 '$_[0]' 2>/dev/null";
	my $jsonhdr = `$cmd`;
	return $jsonhdr;
}

# biosig_fhir: makes a fhir_json_binary template
sub biosig_fhir_json {
	my $cmd = "biosig_fhir$EXT -json '$_[0]' 2>/dev/null";
	my $jsonhdr = `$cmd`;
	return $jsonhdr;
}

# biosig_fhir_xml: makes a fhir_xml_binary template
sub biosig_fhir_xml {
	my $h = "biosig_fhir$EXT -xml $_[0] 2>/dev/null";
	my $jsonhdr = `$h`;
	return $jsonhdr;
}

# get header (meta) information
sub biosig_json_header {
	my $h = "save2gdf$EXT -JSON $_[0] 2>/dev/null";
	my $jsonhdr = `$h`;
	return $jsonhdr;
}

# get header (meta) information in hash array
sub biosig_header {
	return decode_json( biosig_json_header("$_[0]") );
}



my $filename='data/Newtest17-256.bdf';

### Hash array of header information through JSON interface
my $HDR = biosig_header($filename);
print Dumper($HDR);

## generate JSON_BINARY_TEMPLATE
my $FHIR_JSON_BINARY_TEMPLATE = biosig_fhir_json($filename);
print $FHIR_JSON_BINARY_TEMPLATE;


