#!/bin/bash

# Usage: ./misc/make-install.sh
#
# Rebuilds the two subprojects. Use this when you make changes
# and then want to build the main app.
# Use Git Bash or a similar tool to run this on Windows.

cd Alenka-File
for build in `ls build* -d`
do
	cd $build
	cmake --build . --target install-alenka-file &
	cd -
done
cd ..
wait

cd Alenka-Signal
for build in `ls build* -d`
do
        cd $build
        cmake --build . --target install-alenka-signal &
        cd -
done
cd ..
wait

