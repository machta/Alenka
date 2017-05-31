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
#mkdir -p $folder/$name/platforms
#mkdir -p $folder/$name/xcbglintegrations
#mkdir -p $folder/$name/imageformats

cp -v build-Release/Alenka $folder/$name/Alenka && alenka=OK || alenka=fail

QT_DIR=$HOME/Qt/5.8/clang_64

cp -vr $QT_DIR/qml/{Qt,QtQuick,QtQuick.2} $folder/$name &&
qml=OK || qml=fail

#PLUGIN=$QT_DIR/plugins &&
#cp -v $PLUGIN/platforms/libqxcb.so $folder/$name/platforms &&
#cp -v $PLUGIN/xcbglintegrations/* $folder/$name/xcbglintegrations &&
#plugin=OK || plugin=fail

#export LD_LIBRARY_PATH=$QT_DIR/lib &&
#cp -v $(realpath -s `ldd $folder/$name/Alenka.bin $(find $folder/$name -name '*.so') | grep -i qt | grep -v Gamepad | awk '{print $3}'` | sort | uniq) $folder/$name &&
#cp -v $QT_DIR/plugins/imageformats/libqjpeg.so $folder/$name/imageformats &&
#chmod a-x $folder/$name/lib*so* &&
#libraries=OK || libraries=fail

#echo '#!/bin/sh

#DIR=`dirname $0`
#export LD_LIBRARY_PATH=$DIR
#' > $folder/$name/Alenka

#echo '#!/bin/sh

#DIR=`dirname $0`
#export LD_LIBRARY_PATH=$DIR:$AMDAPPSDKROOT/lib/x86_64/sdk
#' > $folder/$name/Alenka-AMD

#echo '$DIR/Alenka.bin "$@"' | tee -a $folder/$name/Alenka >> $folder/$name/Alenka-AMD
#chmod u+x $folder/$name/Alenka*

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
#echo "plugin                  $plugin"
#echo "shared libraries        $libraries"
echo "zip                     $zip"

