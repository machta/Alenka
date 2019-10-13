#!/bin/bash

# Usage: ./misc/deploy/windows.sh fileName
#
# This script makes a standalone ZIP package for distribution on Windows.
#
# Tested on Windows 7/10.
#
# Use Git Bash or a similar tool to run this.
#
# PowerShell 3 and .NET 4 are needed for the archive creation. If you don't have
# these, comment out the last command (rm -r $folder $folder32) and make the
# archive yourself from the temporary directories.

name=$1
if [ "$name" == "" ]
then
  name=Alenka-Windows
fi

folder=`mktemp -d -p .`
folder32=`mktemp -d -p .`
BUILD64=build-Release-64
BUILD32=build-Release-32

echo -e Deploying to $name.zip and $name-32.zip'\n'

# Prepare directory structure.
make_dirs() {
  _folder="$1"
  _name="$2"

  mkdir -p "$_folder/$_name"/log &&
    mkdir -p "$_folder/$_name"/montageTemplates &&
    mkdir -p "$_folder/$_name"/platforms &&
    mkdir -p "$_folder/$_name"/imageformats &&
    mkdir -p "$_folder/$_name"/mediaservice
}

make_dirs "$folder" "$name" &&
  dirs=OK || dirs=fail

make_dirs "$folder32" "$name-32" &&
  dirs32=OK || dirs32=fail

# Copy the exe.
cp -v `find $BUILD64 -name Alenka.exe` "$folder/$name" &&
  alenka=OK || alenka=fail

cp -v `find $BUILD32 -name Alenka.exe` "$folder32/$name-32" &&
  alenka32=OK || alenka32=fail

# Copy all the libraries.
copy_libraries() {
  _folder="$1"
  _name="$2"
  _build="$3"

  cp -v `find $_build -name hdf5.dll | head -1` "$_folder/$_name" &&
    cp -v `find $_build -name libmatio.dll | head -1` "$_folder/$_name" &&
    cp -v `find $_build -name szip.dll | head -1` "$_folder/$_name" &&
    cp -v `find $_build -name zlib.dll | head -1` "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Core.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Gui.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Widgets.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Network.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5WebSockets.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Charts.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Multimedia.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5MultimediaWidgets.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5OpenGL.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Qml.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Quick.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5QuickWidgets.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5QuickControls2.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5QuickTemplates2.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5Svg.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/bin/Qt5XmlPatterns.dll" "$_folder/$_name" &&
    cp -v "$QT_DIR/plugins/platforms/qwindows.dll" "$_folder/$_name/platforms" &&
    cp -v "$QT_DIR/plugins/imageformats/qjpeg.dll" "$_folder/$_name/imageformats" &&
    cp -v "$QT_DIR/plugins/mediaservice/"*.dll "$_folder/$_name/mediaservice" &&
    cp -vr "$QT_DIR"/qml/{Qt,QtQuick,QtQuick.2} "$_folder/$_name"
}

QT_DIR=C:/Qt/5.9.5/msvc2015_64 &&
  copy_libraries "$folder" "$name" "$BUILD64" &&
  libraries=OK || libraries=fail

QT_DIR=C:/Qt/5.9.5/msvc2015 &&
  copy_libraries "$folder32" "$name-32" "$BUILD32" &&
  libraries32=OK || libraries32=fail

# Package additional files.
README="`dirname $0`/readme-windows.txt"
INI="`dirname $0`/options.ini"
HEADER="`dirname $0`/montageHeader.cl"
TEMPLATES="`dirname $0`/montageTemplates"
LIC="`dirname $0`/../../LICENSE.txt"

package_files() {
  _folder="$1"
  _name="$2"

  cp -v "$README" "$_folder/$_name/README.txt" &&
    cp -v "$INI" "$_folder/$_name" &&
    cp -v "$HEADER" "$_folder/$_name" &&
    cp -v "$TEMPLATES"/* "$_folder/$_name/montageTemplates" &&
    cp -v "$LIC" "$_folder/$_name"
}

package_files "$folder" "$name" &&
  package=OK || package=fail

package_files "$folder32" "$name-32" &&
  package32=OK || package32=fail

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
echo "prepare dirs            $dirs"
echo "prepare dirs 32-bit     $dirs32"
echo "Alenka                  $alenka"
echo "Alenka 32-bit           $alenka32"
echo "DLL libraries           $libraries"
echo "DLL libraries 32-bit    $libraries32"
echo "package files           $package"
echo "package files 32-bit    $package32"
echo "zip                     $zip"
