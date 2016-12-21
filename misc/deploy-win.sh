#!/bin/bash

# Usage: ./misc/deploy_unix.sh fileName
#
# This script makes a ZIP archive of the Windows version of the program.
# Use Git Bash or a similar tool to run this.

name=$1
if [ "$name" == "" ]
then
    name=Alenka-Windows
fi

folder=`mktemp -d -p .`
folder32=`mktemp -d -p .`

echo -e Deploying to $name.zip and $name-32.zip'\n'

mkdir -p $folder/$name/platforms
mkdir -p $folder32/$name-32/platforms

cp -v `find .. -name Alenka.exe | grep Alenka | grep 64 | grep Release` $folder/$name/Alenka.exe
cp -v `find .. -name Alenka.exe | grep Alenka | grep 32 | grep Release` $folder32/$name-32/Alenka.exe

QT_DIR=C:/Qt/5.7/msvc2015_64
cp -v $QT_DIR/bin/Qt5Core.dll $folder/$name
cp -v $QT_DIR/bin/Qt5Gui.dll $folder/$name
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder/$name
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder/$name/platforms

QT_DIR=C:/Qt/5.7/msvc2015
cp -v $QT_DIR/bin/Qt5Core.dll $folder32/$name-32
cp -v $QT_DIR/bin/Qt5Gui.dll $folder32/$name-32
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder32/$name-32
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder32/$name-32/platforms

#zip -r $name.zip $name
rm -f "$name.zip" "$name-32.zip"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder', '$name.zip'); }"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder32', '$name-32.zip'); }"

rm -r $folder $folder32

