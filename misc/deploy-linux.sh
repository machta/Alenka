#!/bin/bash

# Usage: ./misc/deploy-linux.sh fileName
#
# This script makes a standalone ZIP package for distribution on Linux.
#
# Tested on Ubuntu 14/16, Debian 8.
# You must build and package the 32-bit version separately (possibly on a different OS).

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name/platforms
mkdir -p $folder/$name/xcbglintegrations

cp -v `find .. -type f -name Alenka | grep Alenka | grep Release | grep 5.7` $folder/$name/Alenka && alenka=OK || alenka=fail
cp -v `find .. -type f -name Alenka | grep Alenka | grep Debug | grep 5.7` $folder/$name/Alenka.debug && alenkaDebug=OK || alenkaDebug=fail

PLUGIN=/opt/Qt/5.7/gcc_64/plugins &&
cp -v $PLUGIN/platforms/libqxcb.so $folder/$name/platforms &&
cp -v $PLUGIN/xcbglintegrations/* $folder/$name/xcbglintegrations &&
cp -v $(realpath -s `ldd $folder/$name/Alenka $PLUGIN/platforms/libqxcb.so | grep -i qt | awk '{print $3}'` | sort | uniq) $folder/$name &&
libraries=OK || libraries=fail

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$AMDAPPSDKROOT/lib/x86_64/sdk:$LD_LIBRARY_PATH
' | tee $folder/$name/runAlenka > $folder/$name/runAlenka.debug
echo '$DIR/Alenka "$@"' >> $folder/$name/runAlenka
echo '$DIR/Alenka.debug "$@"' >> $folder/$name/runAlenka.debug
chmod u+x $folder/$name/run*

echo 'This is a standalone package.

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
echo "shared libraries        $libraries"
echo "zip                     $zip"
