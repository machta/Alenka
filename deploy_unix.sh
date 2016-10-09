#/bin/bash

folder=$1
if [ "$folder" == "" ]
then
folder=./ZSBS_x64
fi

echo Deploying to $folder

mkdir -p $folder

# Assume that the CD is the root of the project
# and that there are two symbolic links to the executables.

cp App/App $folder
cp App/App.debug $folder/App.debug

cp App/*.vert $folder
cp App/*.frag $folder
cp App/*.cl $folder
cp App/edit.png $folder
