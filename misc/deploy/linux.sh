#!/bin/bash

# Usage: ./misc/deploy/linux.sh fileName
#
# This script makes a standalone ZIP package for distribution on Linux.
#
# Tested on Ubuntu 14/16, Debian 8.
# You must build and package the 32-bit version separately (possibly on a
# different OS).

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name/log
mkdir -p $folder/$name/platforms
mkdir -p $folder/$name/xcbglintegrations
mkdir -p $folder/$name/imageformats

cp -v build-Release/Alenka $folder/$name/Alenka.bin && alenka=OK || alenka=fail

QT_DIR=/opt/Qt/5.8/gcc_64

cp -vr $QT_DIR/qml/{Qt,QtQuick,QtQuick.2} $folder/$name &&
qml=OK || qml=fail

PLUGIN=$QT_DIR/plugins &&
cp -v $PLUGIN/platforms/libqxcb.so $folder/$name/platforms &&
cp -v $PLUGIN/xcbglintegrations/* $folder/$name/xcbglintegrations &&
plugin=OK || plugin=fail

export LD_LIBRARY_PATH=$QT_DIR/lib &&
cp -v $(realpath -s `ldd $folder/$name/Alenka.bin $(find $folder/$name -name '*.so') | grep -i qt | grep -v Gamepad | awk '{print $3}'` | sort | uniq) $folder/$name &&
cp -v $QT_DIR/plugins/imageformats/libqjpeg.so $folder/$name/imageformats &&
chmod a-x $folder/$name/lib*so* &&
libraries=OK || libraries=fail

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR
' > $folder/$name/Alenka

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$AMDAPPSDKROOT/lib/x86_64/sdk
' > $folder/$name/Alenka-AMD

echo '$DIR/Alenka.bin "$@"' | tee -a $folder/$name/Alenka >> $folder/$name/Alenka-AMD
chmod u+x $folder/$name/Alenka*

cp -v "`dirname $0`/readme-linux.txt" $folder/$name/README
cp -v "`dirname $0`/options.ini" $folder/$name

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
echo "qml                     $qml"
echo "plugin                  $plugin"
echo "shared libraries        $libraries"
echo "zip                     $zip"

