#!/bin/bash

# Usage: ./misc/deploy-linux.sh fileName
#
# This script makes a standalone ZIP package for distribution on Linux.
# Tested on Ubuntu 14/16.
# You must build and package the 32-bit version separately (possibly on a different OS).

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name/platforms

cp -v `find .. -type f -name Alenka | grep Alenka | grep Release` $folder/$name/Alenka
cp -v `find .. -type f -name Alenka | grep Alenka | grep Debug` $folder/$name/Alenka.debug

PLUGIN=/opt/Qt/5.7/gcc_64/plugins/platforms/libqxcb.so
cp -v $PLUGIN $folder/$name/platforms
cp -v `ldd $folder/$name/Alenka $PLUGIN | grep -i qt | awk '{print $3}'` $folder/$name

echo '#!/bin/sh

DIR=`dirname $0`
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH
export LC_NUMERIC=en_US.UTF-8
' | tee $folder/$name/runAlenka > $folder/$name/runAlenka.debug
echo '$DIR/Alenka "$@"' >> $folder/$name/runAlenka
echo '$DIR/Alenka.debug "$@"' >> $folder/$name/runAlenka.debug
chmod u+x $folder/$name/run*

cd $folder
zip -r $name.zip $name
mv $name.zip ..
cd -

rm -r $folder

#TODO: make a README with installation instructions
