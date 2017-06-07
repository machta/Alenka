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
mkdir -p $folder/$name/platforms
mkdir -p $folder/$name/imageformats
mkdir -p $folder/$name/Qt/labs/folderlistmodel
mkdir -p $folder/$name/Qt/labs/settings
mkdir -p $folder/$name/QtQuick/Controls
mkdir -p $folder/$name/QtQuick/Controls.2
mkdir -p $folder/$name/QtQuick/Dialogs
mkdir -p $folder/$name/QtQuick/Dialogs/Private
mkdir -p $folder/$name/QtQuick/Dialogs/qml
mkdir -p $folder/$name/QtQuick/Layouts
mkdir -p $folder/$name/QtQuick/Templates.2
mkdir -p $folder/$name/QtQuick/Window.2
mkdir -p $folder/$name/QtQuick/XmlListModel
mkdir -p $folder/$name/QtQuick.2

mkdir -p $folder32/$name-32/log
mkdir -p $folder32/$name-32/platforms
mkdir -p $folder32/$name-32/imageformats
mkdir -p $folder32/$name-32/Qt/labs/folderlistmodel
mkdir -p $folder32/$name-32/Qt/labs/settings
mkdir -p $folder32/$name-32/QtQuick/Controls
mkdir -p $folder32/$name-32/QtQuick/Controls.2
mkdir -p $folder32/$name-32/QtQuick/Dialogs
mkdir -p $folder32/$name-32/QtQuick/Dialogs/Private
mkdir -p $folder32/$name-32/QtQuick/Dialogs/qml
mkdir -p $folder32/$name-32/QtQuick/Layouts
mkdir -p $folder32/$name-32/QtQuick/Templates.2
mkdir -p $folder32/$name-32/QtQuick/Window.2
mkdir -p $folder32/$name-32/QtQuick/XmlListModel
mkdir -p $folder32/$name-32/QtQuick.2

BUILD64=build-Release-64
BUILD32=build-Release-32

cp -v `find $BUILD64 -name Alenka.exe` $folder/$name && alenka=OK || alenka=fail
cp -v `find $BUILD32 -name Alenka.exe` $folder32/$name-32 && alenka32=OK || alenka32=fail

QT_DIR=C:/Qt/5.8/msvc2015_64 &&
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
cp -v $QT_DIR/plugins/platforms/qwindows.dll $folder/$name/platforms &&
cp -v $QT_DIR/bin/Qt5Qml.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Quick.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickWidgets.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickControls2.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5QuickTemplates2.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5Svg.dll $folder/$name &&
cp -v $QT_DIR/bin/Qt5XmlPatterns.dll $folder/$name &&
cp -v $QT_DIR/plugins/imageformats/qjpeg.dll $folder/$name/imageformats &&
cp -v $QT_DIR/qml/Qt/labs/folderlistmodel/qmlfolderlistmodelplugin.dll $folder/$name/Qt/labs/folderlistmodel &&
cp -v $QT_DIR/qml/Qt/labs/folderlistmodel/qmldir $folder/$name/Qt/labs/folderlistmodel &&
cp -v $QT_DIR/qml/Qt/labs/settings/qmlsettingsplugin.dll $folder/$name/Qt/labs/settings &&
cp -v $QT_DIR/qml/Qt/labs/settings/qmldir $folder/$name/Qt/labs/settings &&
cp -v $QT_DIR/qml/QtQuick/Controls/qtquickcontrolsplugin.dll $folder/$name/QtQuick/Controls &&
cp -v $QT_DIR/qml/QtQuick/Controls/Splitview.qml $folder/$name/QtQuick/Controls &&
cp -v $QT_DIR/qml/QtQuick/Controls/qmldir $folder/$name/QtQuick/Controls &&
cp -R $QT_DIR/qml/QtQuick/Controls.2 $folder/$name/QtQuick &&
rm $folder/$name/QtQuick/Controls.2/qtquickcontrols2plugind.pdb &&
rm $folder/$name/QtQuick/Controls.2/qtquickcontrols2plugind.dll &&
rm -R $folder/$name/QtQuick/Controls.2/Material &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/dialogplugin.dll $folder/$name/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/DefaultFileDialog.qml $folder/$name/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qmldir $folder/$name/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/ColorSlider.qml $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/DefaultWindowDecoration.qml $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/IconButtonStyle.qml $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/IconGlyph.qml $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/icons.ttf $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/qmldir $folder/$name/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/Private/dialogsprivateplugin.dll $folder/$name/QtQuick/Dialogs/Private &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/Private/qmldir $folder/$name/QtQuick/Dialogs/Private &&
cp -v $QT_DIR/qml/QtQuick/Layouts/qquicklayoutsplugin.dll $folder/$name/QtQuick/Layouts &&
cp -v $QT_DIR/qml/QtQuick/Layouts/qmldir $folder/$name/QtQuick/Layouts &&
cp -v $QT_DIR/qml/QtQuick/Templates.2/qtquicktemplates2plugin.dll $folder/$name/QtQuick/Templates.2 &&
cp -v $QT_DIR/qml/QtQuick/Templates.2/qmldir $folder/$name/QtQuick/Templates.2 &&
cp -v $QT_DIR/qml/QtQuick/Window.2/windowplugin.dll $folder/$name/QtQuick/Window.2 &&
cp -v $QT_DIR/qml/QtQuick/Window.2/qmldir $folder/$name/QtQuick/Window.2 &&
cp -v $QT_DIR/qml/QtQuick/XmlListModel/qmlxmllistmodelplugin.dll $folder/$name/QtQuick/XmlListModel &&
cp -v $QT_DIR/qml/QtQuick/XmlListModel/qmldir $folder/$name/QtQuick/XmlListModel &&
cp -v $QT_DIR/qml/QtQuick.2/qtquick2plugin.dll $folder/$name/QtQuick.2 &&
cp -v $QT_DIR/qml/QtQuick.2/qmldir $folder/$name/QtQuick.2 &&
libraries=OK || libraries=fail

QT_DIR=C:/Qt/5.8/msvc2015 &&
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
cp -v $QT_DIR/qml/Qt/labs/folderlistmodel/qmlfolderlistmodelplugin.dll $folder32/$name-32/Qt/labs/folderlistmodel &&
cp -v $QT_DIR/qml/Qt/labs/folderlistmodel/qmldir $folder32/$name-32/Qt/labs/folderlistmodel &&
cp -v $QT_DIR/qml/Qt/labs/settings/qmlsettingsplugin.dll $folder32/$name-32/Qt/labs/settings &&
cp -v $QT_DIR/qml/Qt/labs/settings/qmldir $folder32/$name-32/Qt/labs/settings &&
cp -v $QT_DIR/qml/QtQuick/Controls/qtquickcontrolsplugin.dll $folder32/$name-32/QtQuick/Controls &&
cp -v $QT_DIR/qml/QtQuick/Controls/Splitview.qml $folder32/$name-32/QtQuick/Controls &&
cp -v $QT_DIR/qml/QtQuick/Controls/qmldir $folder32/$name-32/QtQuick/Controls &&
cp -R $QT_DIR/qml/QtQuick/Controls.2 $folder32/$name-32/QtQuick&&
rm $folder32/$name-32/QtQuick/Controls.2/qtquickcontrols2plugind.pdb &&
rm $folder32/$name-32/QtQuick/Controls.2/qtquickcontrols2plugind.dll &&
rm -R $folder32/$name-32/QtQuick/Controls.2/Material &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/dialogplugin.dll $folder32/$name-32/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/DefaultFileDialog.qml $folder32/$name-32/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qmldir $folder32/$name-32/QtQuick/Dialogs &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/ColorSlider.qml $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/DefaultWindowDecoration.qml $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/IconButtonStyle.qml $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/IconGlyph.qml $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/icons.ttf $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/qml/qmldir $folder32/$name-32/QtQuick/Dialogs/qml &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/Private/dialogsprivateplugin.dll $folder32/$name-32/QtQuick/Dialogs/Private &&
cp -v $QT_DIR/qml/QtQuick/Dialogs/Private/qmldir $folder32/$name-32/QtQuick/Dialogs/Private &&
cp -v $QT_DIR/qml/QtQuick/Layouts/qquicklayoutsplugin.dll $folder32/$name-32/QtQuick/Layouts &&
cp -v $QT_DIR/qml/QtQuick/Layouts/qmldir $folder32/$name-32/QtQuick/Layouts &&
cp -v $QT_DIR/qml/QtQuick/Templates.2/qtquicktemplates2plugin.dll $folder32/$name-32/QtQuick/Templates.2 &&
cp -v $QT_DIR/qml/QtQuick/Templates.2/qmldir $folder32/$name-32/QtQuick/Templates.2 &&
cp -v $QT_DIR/qml/QtQuick/Window.2/windowplugin.dll $folder32/$name-32/QtQuick/Window.2 &&
cp -v $QT_DIR/qml/QtQuick/Window.2/qmldir $folder32/$name-32/QtQuick/Window.2 &&
cp -v $QT_DIR/qml/QtQuick/XmlListModel/qmlxmllistmodelplugin.dll $folder32/$name-32/QtQuick/XmlListModel &&
cp -v $QT_DIR/qml/QtQuick/XmlListModel/qmldir $folder32/$name-32/QtQuick/XmlListModel &&
cp -v $QT_DIR/qml/QtQuick.2/qtquick2plugin.dll $folder32/$name-32/QtQuick.2 &&
cp -v $QT_DIR/qml/QtQuick.2/qmldir $folder32/$name-32/QtQuick.2 &&
libraries32=OK || libraries32=fail

README="`dirname $0`/readme-windows.txt"
cp -v "$README" $folder/$name/README.txt
cp -v "$README" $folder32/$name-32/README.txt

INI="`dirname $0`/options.ini"
cp -v "$INI" $folder/$name
cp -v "$INI" $folder32/$name-32

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

