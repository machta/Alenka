#!/bin/bash

# Usage: ./misc/make-install.sh
#
# This script pulls and rebuilds the two subprojects.
# Use this when you make changes there and then want to build the main app.
# Use Git Bash or a similar tool to run this on Windows.

cd Alenka-File && git pull && fpull="OK" || fpull="fail"
cd ..
cd Alenka-Signal && git pull && spull="OK" || spull="fail"
cd ..

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
echo ================= Pull summary ==================
echo "Library path                             Status"
echo =================================================
echo "Alenka-File                              $fpull"
echo "Alenka-Signal                            $spull"
echo

echo ================= Build summary =================
echo "Build directory path                     Status"
echo =================================================
echo -en "$SUMMARY"

