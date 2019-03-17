#!/bin/bash

# Usage: ./misc/deploy/windows.sh fileName
#
# This script makes a standalone ZIP package for distribution on Windows.
#
# Tested on Windows 7/10.
#
# Use Git Bash or a similar tool to run this.
#
# PowerShell 3 and .NET 4 is needed for the archive creation. If you don't have
# these, comment out the last command (rm -r $folder $folder32) and make the
# archive yourself from the temporary directories.

name=$1
if [ "$name" == "" ]
then
  name=Alenka-Windows
fi

folder=`mktemp -d -p .`
folder32=`mktemp -d -p .`

echo -e Deploying to $name.zip and $name-32.zip'\n'

mkdir -p $folder/$name/log
mkdir -p $folder/$name/montageTemplates
mkdir -p $folder/$name/platforms
mkdir -p $folder/$name/imageformats

mkdir -p $folder32/$name-32/log
mkdir -p $folder32/$name-32/montageTemplates
mkdir -p $folder32/$name-32/platforms
mkdir -p $folder32/$name-32/imageformats

BUILD64=build-Release-64
BUILD32=build-Release-32

cp -v `find $BUILD64 -name Alenka.exe` $folder/$name && alenka=OK || alenka=fail
cp -v `find $BUILD32 -name Alenka.exe` $folder32/$name-32 && alenka32=OK || alenka32=fail

QT_DIR=C:/Qt/5.9.5/msvc2015_64 &&
cp -v `find $BUILD64 -name hdf5.dll | head -1` $folder/$name &&
cp -v `find $BUILD64 -name libmatio.dll | head -1` $folder/$name &&
cp -v `find $BUILD64 -name szip.dll | head -1` $folder/$name &&
cp -v `find $BUILD64 -name zlib.dll | head -1` $folder/$name &&
cp -v $QT_DIR/bin/Qt5Core.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Gui.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Network.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5WebSockets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Charts.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Multimedia.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5MultimediaWidgets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5OpenGL.dll $folder/$name &&
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder/$name/platforms &&
cp -v $QT_DIR/bin/Qt5Qml.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Quick.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickWidgets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickControls2.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickTemplates2.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Svg.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5XmlPatterns.dll $folder/$name &&
cp -v $QT_DIR/plugins/imageformats/qjpeg.dll $folder/$name/imageformats &&
cp -vr $QT_DIR/qml/{Qt,QtQuick,QtQuick.2} $folder/$name &&
libraries=OK || libraries=fail

QT_DIR=C:/Qt/5.9.5/msvc2015 &&
cp -v `find $BUILD32 -name hdf5.dll | head -1` $folder32/$name-32 &&
cp -v `find $BUILD32 -name libmatio.dll | head -1` $folder32/$name-32 &&
cp -v `find $BUILD32 -name szip.dll | head -1` $folder32/$name-32 &&
cp -v `find $BUILD32 -name zlib.dll | head -1` $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Core.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Gui.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Network.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5WebSockets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Charts.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Multimedia.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5MultimediaWidgets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5OpenGL.dll $folder32/$name-32 &&
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder32/$name-32/platforms &&
cp -v $QT_DIR/bin/Qt5Qml.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5QuickWidgets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Quick.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5QuickControls2.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5QuickTemplates2.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Svg.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5Widgets.dll $folder32/$name-32 &&
cp -v $QT_DIR/bin/Qt5XmlPatterns.dll $folder32/$name-32 &&
cp -v $QT_DIR/plugins/imageformats/qjpeg.dll $folder32/$name-32/imageformats &&
cp -vr $QT_DIR/qml/{Qt,QtQuick,QtQuick.2} $folder32/$name-32 &&
libraries32=OK || libraries32=fail

README="`dirname $0`/readme-windows.txt"
cp -v "$README" $folder/$name/README.txt
cp -v "$README" $folder32/$name-32/README.txt

INI="`dirname $0`/options.ini"
cp -v "$INI" $folder/$name
cp -v "$INI" $folder32/$name-32

HEADER="`dirname $0`/montageHeader.cl"
cp -v "$HEADER" $folder/$name
cp -v "$HEADER" $folder32/$name-32

TEMPLATES="`dirname $0`/montageTemplates"
cp -v "$TEMPLATES"/* $folder/$name/montageTemplates
cp -v "$TEMPLATES"/* $folder32/$name-32/montageTemplates

LIC="`dirname $0`/../../LICENSE.txt"
cp -v "$LIC" $folder/$name
cp -v "$LIC" $folder32/$name-32

# Make zip using .Net.
rm -f "$name.zip" "$name-32.zip" &&
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder', '$name.zip'); }" &&
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('$folder32', '$name-32.zip'); }" &&
zip=OK || zip=fail

rm -r $folder $folder32

echo
echo ========= Deployment summary =========
echo "Files                   Status"
echo ======================================
echo "Alenka                  $alenka"
echo "Alenka 32-bit           $alenka32"
echo "DLL libraries           $libraries"
echo "DLL libraries 32-bit    $libraries32"
echo "zip                     $zip"

