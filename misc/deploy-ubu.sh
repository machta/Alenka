#!/bin/bash

# Usage: ./misc/deploy-ubu.sh fileName
#
# This script makes a ZIP package for distribution on one version of OS.
# You must build and package the 32-bit version separately (possibly on a different OS).

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Ubu
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name

cp -v `find .. -type f -name Alenka | grep Alenka | grep Release` $folder/$name/Alenka
cp -v `find .. -type f -name Alenka | grep Alenka | grep Debug` $folder/$name/Alenka.debug

echo '#!/bin/sh

DIR=`dirname $0`
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

