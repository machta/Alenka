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

# Download.

if [ -d alglib-3.10.0 ]
then
	alglib=skipped
else
	curl http://www.alglib.net/translator/re/alglib-3.10.0.cpp.gpl.zip > alglib.zip &&
	unzip -q alglib.zip -dalglib-3.10.0 &&
	rm alglib.zip &&
	mv alglib-3.10.0/cpp/* alglib-3.10.0 &&
	rm -r alglib-3.10.0/cpp &&
	alglib=OK || alglib=fail
fi

if [ -d boost_1_63 ]
then
	boost=skipped
else
	BB=boost_1_63_0 &&
	B=boost_1_63 &&
	curl -L https://sourceforge.net/projects/boost/files/boost/1.63.0/$BB.zip > $BB.zip &&
	unzip -q $BB.zip &&
	rm $BB.zip &&
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
	curl -L 'https://sourceforge.net/projects/alenka-mirror/files/misc/matio-msvc2015.zip/download' > matio-msvc2015.zip && 
	unzip -q matio-msvc2015.zip &&
	rm -rf matio-msvc2015.zip &&
	matio=OK || matio=fail
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
echo "boost_1_63              $boost"
echo "matio-msvc2015          $matio"
echo
echo ======= Configuration summary ========
echo "Library path            Status"
echo ======================================
echo "libsamplerate           $libsamplerate"

