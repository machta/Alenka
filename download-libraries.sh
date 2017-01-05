#!/bin/bash

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
echo "Library                 Status"
echo ======================================
echo "Alenka-File             $file"
echo "Alenka-Signal           $signal"
