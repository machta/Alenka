#!/bin/bash

# Usage: ./download-libraries.sh
#
# This script downloads all dependant libraries. Make sure you run it from the
# libraries directory. If you want to redownload a library, just remove the
# appropriate directory.
#
# Use Git Bash or a similar tool to use this on Windows.
#
# TODO: Switch clFFT to master after my locale fix is merged.

git submodule update --init && submodule=OK || submodule=fail

downloadZip()
{
	curl -L $1 > zip.zip &&
	unzip -q zip.zip &&
	rm -f zip.zip
}

# Download.

if [ -d alglib-3.10.0 ]
then
	alglib=skipped
else
	downloadZip http://www.alglib.net/translator/re/alglib-3.10.0.cpp.gpl.zip &&
	mkdir alglib-3.10.0 &&
	mv cpp/* alglib-3.10.0 &&
	rmdir cpp &&
	alglib=OK || alglib=fail
fi

BB=boost_1_66_0 &&
B=boost_1_66 &&
if [ -d $B ]
then
	boost=skipped
else
	downloadZip https://sourceforge.net/projects/boost/files/boost/1.66.0/$BB.zip &&
	mkdir -p $B/libs &&
	mv $BB/boost $B &&
	mv $BB/libs/program_options $B/libs &&
	mv $BB/libs/system $B/libs &&
	mv $BB/libs/filesystem $B/libs &&
	rm -rf $BB &&
	boost=OK || boost=fail
fi

if [ -d matio-msvc2015 ]
then
	matio=skipped
else
	downloadZip https://sourceforge.net/projects/alenka-mirror/files/misc/matio-msvc2015.zip/download &&
	matio=OK || matio=fail
fi

if [ -d libeep-3.3.177 ]
then
	libeep=skipped
else
	downloadZip https://sourceforge.net/projects/libeep/files/libeep-3.3.177.zip/download &&
	libeep=OK || libeep=fail
fi

# Configuration.

cd libsamplerate && # TODO: Perhaps it doesn't need to be run again, once the header is created.
./autogen.sh || cp -v Win32/config.h src &&
cd - &&
libsamplerate=OK || libsamplerate=fail

echo
echo =========== Git submodules ===========
git submodule status
echo "Status: $submodule"
echo
echo ========== Download summary ==========
echo "Library path            Status"
echo ======================================
echo "alglib-3.10.0           $alglib"
echo "$B              $boost"
echo "matio-msvc2015          $matio"
echo "libeep                  $libeep"
echo
echo ======= Configuration summary ========
echo "Library path            Status"
echo ======================================
echo "libsamplerate           $libsamplerate"

