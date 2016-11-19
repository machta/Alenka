#/bin/bash

# Usage: ./deploy_unix.sh fileName
#
# Output is fileName.zip in the current directory.
# Use Git Bash or a similar tool to run this.

name=$1
if [ "$name" == "" ]
then
name=ZSBS-Windows
fi

folder=`mktemp -d -p .`

echo -e Deploying to $name.zip'\n'

mkdir -p $folder/$name

cp -v `find .. -name App.exe | grep 64 | grep Release` $folder/$name/App.exe
cp -v `find .. -name App.exe | grep 32 | grep Release` $folder/$name/App-32.exe

#zip -r $name.zip $name
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder', '$name.zip'); }"

rm -r $folder

