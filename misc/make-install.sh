#!/bin/bash

# Usage: ./misc/make-install.sh
#
# This script rebuilds the two subprojects.
# Use this when you make changes there and then want to build the main app.
# Use Git Bash or a similar tool to run this on Windows.

export MAKEFLAGS=-j4 # Change this to controll how many parallel jobs make can use.

cd Alenka-File
for build in `ls build* -d`
do
	echo Building Alenka-File/$build
	cd $build
	if [ `echo $build | grep Release` ]
	then
		cmake --build . --config Release --target install-alenka-file && STATUS=OK || STATUS=fail
	else
		cmake --build . --target install-alenka-file && STATUS=OK || STATUS=fail
	fi
	cd -
	SUMMARY=$SUMMARY`printf '%-40s %s' Alenka-File/$build $STATUS`'\n'
	echo
done
cd ..

cd Alenka-Signal
for build in `ls build* -d`
do
	echo Building Alenka-Signal/$build
	cd $build
	if [ `echo $build | grep Release` ]
	then
		cmake --build . --config Release --target install-alenka-signal && STATUS=OK || STATUS=fail
	else
		cmake --build . --target install-alenka-signal && STATUS=OK || STATUS=fail
	fi
	cd -
	SUMMARY=$SUMMARY`printf '%-40s %s' Alenka-Signal/$build $STATUS`'\n'
	echo
done
cd ..

echo
echo ================= Build summary =================
echo "Library                                  Status"
echo =================================================
echo -en "$SUMMARY"
