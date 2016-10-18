:: The first parameter is the folder to which to deploy.
:: If empty, use current directory.

set folder=%1
if [%folder%] == [] set folder=.\ZSBS-Windows-x64

echo Deploying to %folder%

::del /q /s %folder%\*
mkdir %folder%

:: App exe and shaders and cl sources
copy App\App.exe %folder%
copy App\*.vert %folder%
copy App\*.frag %folder%
copy App\kernels.cl %folder%
copy App\montageHeader.cl %folder%

:: icon
copy App\edit.png %folder%

:: clFFT lib
copy App\clFFT.dll %folder%

:: Qt dlls
set qt_dir=C:\Qt\5.7\msvc2015_64

copy %qt_dir%\bin\Qt5Core.dll %folder%
copy %qt_dir%\bin\Qt5Gui.dll %folder%
copy %qt_dir%\bin\icu*.dll %folder%
copy %qt_dir%\bin\Qt5Widgets.dll %folder%

mkdir %folder%\platforms
copy %qt_dir%\plugins\platforms\qwindows.dll %folder%\platforms

