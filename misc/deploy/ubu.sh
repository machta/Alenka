#!/bin/bash

# Usage: ./misc/deploy/ubu.sh fileName
#
# Use this script to make a ZIP package for Ubuntu 16 that has preinstalled Qt 5.5.
#
# You must build and package the 32-bit version separately (possibly on a different OS).

echo "This deployment script is BROKEN. Don't use it."
echo "Qt 5.5 is not supported at the moment."
exit

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Ubu
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name

cp -v `find .. -type f -name Alenka | grep Alenka | grep Release | grep 5.5` $folder/$name/Alenka && alenka=OK || alenka=fail
cp -v `find .. -type f -name Alenka | grep Alenka | grep Debug | grep 5.5` $folder/$name/Alenka.debug && alenkaDebug=OK || alenkaDebug=fail

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$AMDAPPSDKROOT/lib/x86_64/sdk:$LD_LIBRARY_PATH
' | tee $folder/$name/runAlenka > $folder/$name/runAlenka.debug
echo '$DIR/Alenka "$@"' >> $folder/$name/runAlenka
echo '$DIR/Alenka.debug "$@"' >> $folder/$name/runAlenka.debug
chmod u+x $folder/$name/run*

echo 'This version requires Qt 5.5 shared libraries to be installed.
For Ubuntu 16 you can do that by installing appropriate packages (usualy they are already installed).
For Ubuntu 14 use the installer from Qt website to install package "Qt 5.5 -- Desktop gcc".

Use "./runAlenka" to launch the program.
' > $folder/$name/README

cd $folder &&
zip -r $name.zip $name &&
mv $name.zip .. &&
cd - && zip=OK || zip=fail

rm -r $folder

echo
echo ========= Deployment summary =========
echo "Files                   Status"
echo ======================================
echo "Alenka                  $alenka"
echo "Alenka.debug            $alenkaDebug"
echo "zip                     $zip"
