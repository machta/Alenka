#/bin/bash

# Usage: ./deploy_unix.sh fileName
#
# This script make a ZIP archive of the Linux version of the program.
# Output is fileName.zip in the current directory.
# Build and package the 32-bit version separately.

name=$1
if [ "$name" == "" ]
then
	name=ZSBS-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name'\n'

mkdir -p $folder/$name

cp -v `find .. -type f -name App | grep ZSBS | grep Release` $folder/$name/App
cp -v `find .. -type f -name App | grep ZSBS | grep Debug` $folder/$name/App.debug

cd $folder
zip -r $name.zip $name
mv $name.zip ..
cd -

rm -r $folder

