#/bin/bash

# Usage: ./deploy_unix.sh fileName
# Output is fileName.zip in the current directory.

name=$1
if [ "$name" == "" ]
then
name=ZSBS-Linux
fi

folder=`mktemp -d -p .`

echo -e Deploying to $folder'\n'

mkdir -p $folder/$name

cp -v `find .. -type f -name App | grep 64 | grep Release` $folder/$name/App
cp -v `find .. -type f -name App | grep 64 | grep Debug` $folder/$name/App.debug

cp -v `find .. -type f -name App | grep 32 | grep Release` $folder/$name/App
cp -v `find .. -type f -name App | grep 32 | grep Debug` $folder/$name/App.debug

cd $folder
zip -r $name.zip $name
mv $name.zip ..
cd -
rm -r $folder

