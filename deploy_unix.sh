#/bin/bash

folder=$1
if [ "$folder" == "" ]
then
folder=./ZSBS-Linux-x64
fi

echo -e Deploying to $folder'\n'

mkdir -p $folder

cp -v App/App $folder
cp -v App/App.debug $folder
cp -v App/*.vert $folder
cp -v App/*.frag $folder
cp -v App/*.cl $folder
cp -v App/edit.png $folder
