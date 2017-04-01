#!/bin/bash

# Usage: ./misc/deploy-windows.sh fileName
#
# This script makes a standalone ZIP package for distribution on Windows.
# Tested on Windows 7/10.
# Use Git Bash or a similar tool to run this. This requires .NET for the archive creation.
# If you don't have that, comment out the last line and make it manually.

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

cp -v `find .. -name Alenka.exe | grep Alenka | grep 64 | grep Release | grep 5.7` $folder/$name/Alenka.exe && alenka=OK || alenka=fail
cp -v `find .. -name Alenka.exe | grep Alenka | grep 32 | grep Release | grep 5.7` $folder32/$name-32/Alenka.exe && alenka32=OK || alenka32=fail

QT_DIR=C:/Qt/5.7/msvc2015_64 &&
cp -v $QT_DIR/bin/Qt5Core.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Gui.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Network.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5WebSockets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Charts.dll $folder/$name &&
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder/$name/platforms &&
libraries=OK || libraries=fail

QT_DIR=C:/Qt/5.7/msvc2015 &&
cp -v $QT_DIR/bin/Qt5Core.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Gui.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Network.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5WebSockets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Charts.dll $folder32/$name-32 &&
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder32/$name-32/platforms &&
libraries32=OK || libraries32=fail

README='Visual C++ 2015 redistributable is required.\r
\r
Use "./Alenka" to launch the program or double-click.\r
'
echo -e "$README" > $folder/$name/README.txt
echo -e "$README" > $folder32/$name-32/README.txt

# Make zip using .Net.
rm -f "$name.zip" "$name-32.zip" &&
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder', '$name.zip'); }" &&
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder32', '$name-32.zip'); }" &&
zip=OK || zip=fail

rm -r $folder $folder32

echo
echo ========= Deployment summary =========
echo "Library                 Status"
echo ======================================
echo "Alenka                  $alenka"
echo "Alenka 32-bit           $alenka32"
echo "DLL libraries           $libraries"
echo "DLL libraries 32-bit    $libraries32"
echo "zip                     $zip"
