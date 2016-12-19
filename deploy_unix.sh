#/bin/bash

# Usage: ./deploy_unix.sh fileName
#
# This script make a ZIP archive of the Linux version of the program.
# Output is fileName.zip in the current directory.
# Build and package the 32-bit version separately.

name=$1
if [ "$name" == "" ]
then
	name=Alenka-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name

cp -v `find .. -type f -name Alenka | grep Alenka | grep Release` $folder/$name/Alenka
cp -v `find .. -type f -name Alenka | grep Alenka | grep Debug` $folder/$name/Alenka.debug

cd $folder
zip -r $name.zip $name
mv $name.zip ..
cd -

rm -r $folder

