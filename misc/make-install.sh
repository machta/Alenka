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
	cd $build
	if [ `echo $build | grep Release` ]
	then
		cmake --build . --config Release --target install-alenka-file
	else
		cmake --build . --target install-alenka-file
	fi
	cd -
done
cd ..

cd Alenka-Signal
for build in `ls build* -d`
do
	cd $build
	if [ `echo $build | grep Release` ]
	then
		cmake --build . --config Release --target install-alenka-signal
	else
		cmake --build . --target install-alenka-signal
	fi
	cd -
done
cd ..
