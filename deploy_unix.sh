#/bin/bash

folder=$1
if [ "$folder" == "" ]
then
folder=./ZSBS-Linux-x64
fi

echo -e Deploying to $folder'\n'

mkdir -p $folder

# Assume that the CD is the root of the project
# and that there are two symbolic links to the executables.

cp -v App/App $folder
cp -v App/App.debug $folder

cp -v App/*.vert $folder
cp -v App/*.frag $folder
cp -v App/*.cl $folder
cp -v App/edit.png $folder
