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
cp -v misc/alenkaSave.m $folder/$name &&
libraries=OK || libraries=fail

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH
' | tee $folder/$name/runAlenka > $folder/$name/runAlenka.debug

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$AMDAPPSDKROOT/lib/x86_64/sdk:$LD_LIBRARY_PATH
' | tee $folder/$name/runAlenka-AMD > $folder/$name/runAlenka-AMD.debug

echo '$DIR/Alenka "$@"' | tee -a $folder/$name/runAlenka >> $folder/$name/runAlenka-AMD
echo '$DIR/Alenka.debug "$@"' | tee -a $folder/$name/runAlenka.debug >> $folder/$name/runAlenka-AMD.debug
chmod u+x $folder/$name/run*

echo \
'This is a standalone Linux package, and no additional software should be needed.

You need to install the OpenCL driver (libOpenCL.so) and the implementation
appropriate for your system. If you have a dedicated GPU, install the driver
provided by the manufacturer. That should be enough. On Ubuntu the driver is
provided by a package called "ocl-icd-libopencl1".

For Intel integrated GPUs use a package called "beignet".

With Mesa OpenGL driver (the default for most Linux distributions) you must set
"glSharing = 0". (Apparently it doesn'"'"'t support the cl_khr_gl_sharing extension
due to a bug).

If any of these doesn'"'"'t work for you, you can try AMD APP SDK which provides CPU
only implementation of OpenCL. This is also the only way to run this on
VirtualBox. Don'"'"'t forget to install the VB Guest Additions.

Use "./runAlenka" to launch the program.
Use "./runAlenka-AMD" to override the default OpenCL driver so that AMD APP SDK
can be found.

Use --help to get a list of all the available options.

You can use "alenkaSave.m" to export a Matlab matrix to a MAT file in the format
that can then be opend by Alenka.
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

