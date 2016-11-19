#/bin/bash -x

name=$1
if [ "$name" == "" ]
then
name=ZSBS-Linux-x64
fi

folder=`mktemp -d -p .`

echo -e Deploying to $folder'\n'

mkdir -p $folder/$name

# Assume that the CD is the root of the project
# and that there are two symbolic links to the executables.

cp -v App/App $folder/$name
cp -v App/App.debug $folder/$name

cd $folder
zip -r $name.zip $name
mv $name.zip ..
cd -
rm -r $folder

