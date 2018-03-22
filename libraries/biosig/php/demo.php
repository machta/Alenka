#!/usr/bin/php 
<?php

/*
#
# Demonstrates how the Header information of biosig data can be read through
#    JSON header export
#
# References:
# [1] https://www.hl7.org/fhir/references.html
# [2] ÖNorm K2204:2015, GDF - A general data format for biomedical signals

   Copyright (C) 2016 Alois Schloegl <alois.schloegl@ist.ac.at>
   This file is part of the BioSig repository at http://biosig.sf.net/

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



function ext() {
	if (preg_match("/windows/i",php_uname())) return ".exe";

	return;
}

/*
	biosig_fhir_base64:
		encodes a biosig file as a base64-encoded GDFv3.0 stream
*/
function biosig_fhir_base64($filename) {
	$h = popen('biosig_fhir' . ext() . ' -base64 "' . $filename.'"', 'rb');
	$jsonhdr = stream_get_contents($h);
	fclose($h);
	return $jsonhdr;
}

/*
	biosig_fhir:
		makes a fhir_json_binary template
*/
function biosig_fhir_json($filename) {
	$h = popen('biosig_fhir' . ext() . ' -json "' . $filename.'"', 'rb');
	$jsonhdr = stream_get_contents($h);
	fclose($h);
	return $jsonhdr;
}

/*
	biosig_fhir_xml:
		makes a fhir_xml_binary template
*/
function biosig_fhir_xml($filename) {
	$h = popen('biosig_fhir' . ext() . ' -xml "' . $filename.'"', 'rb');
	$jsonhdr = stream_get_contents($h);
	fclose($h);
	return $jsonhdr;
}

/*
        get header (meta) information
 */
function biosig_json_header($filename) {
	$h = popen('save2gdf' . ext() . ' -JSON "' . $filename.'"', 'rb');
	$jsonhdr = stream_get_contents($h);
	fclose($h);
	return $jsonhdr;
}

/*
        get header (meta) information in php array
*/
function biosig_header($filename) {
	return json_decode( biosig_json_header($filename) );
}

## example file
$filename="data/Newtest17-256.bdf";

## extract header information through JSON export
$HDR = biosig_header($filename);
var_dump($HDR);

## extract fhir binary template
$BIN = biosig_fhir_base64($filename);
echo $BIN;
print "\n\n";

## extract fhir binary template
$BIN = biosig_fhir_json($filename);
echo $BIN;
print "\n\n";
?>

