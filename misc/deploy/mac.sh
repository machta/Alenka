#!/bin/bash

# Usage: ./misc/deploy/mac.sh fileName
#
# This script makes a standalone ZIP package for distribution on Mac OS.
#
# Tested on .

name=$1
if [ "$name" == "" ]
then
  name=Alenka-Mac
fi

folder=`mktemp -d`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name/log

cp -v build-Release/Alenka $folder/$name/Alenka && alenka=OK || alenka=fail

QT_DIR=$HOME/Qt/5.8/clang_64

cp -vr $QT_DIR/qml/{Qt,QtQuick,QtQuick.2} $folder/$name &&
qml=OK || qml=fail

cp -v "`dirname $0`/readme-mac.txt" $folder/$name/README
cp -v "`dirname $0`/options.ini" $folder/$name

cd $folder &&
zip -r $name.zip $name &&
cd - &&
mv $folder/$name.zip . &&
zip=OK || zip=fail

rm -r $folder

echo
echo ========= Deployment summary =========
echo "Files                   Status"
echo ======================================
echo "Alenka                  $alenka"
echo "qml                     $qml"
echo "zip                     $zip"

