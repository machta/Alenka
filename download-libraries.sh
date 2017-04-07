#!/bin/bash

# Usage: ./download-libraries.sh
#
# This script downloads all dependant libraries.
# Use Git Bash or a similar tool to run this on Windows.

if [ -d Alenka-File ]
then
	file=skipped
else
	git clone https://github.com/machta/Alenka-File &&
	file=OK || file=fail
fi

if [ -d Alenka-Signal ]
then
	signal=skipped
else
	git clone https://github.com/machta/Alenka-Signal &&
	signal=OK || signal=fail
fi

echo
echo ========== Download summary ==========
echo "Library path            Status"
echo ======================================
echo "Alenka-File             $file"
echo "Alenka-Signal           $signal"

