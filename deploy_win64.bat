:: The first parameter is the folder to which to deploy.
:: If empty, use current directory.

set folder=%1
if [%folder%] == [] set folder=.\ZSBS_x64

echo Deploying to %folder%

::del /q /s %folder%\*
mkdir %folder%

:: App exe and shaders and cl sources
copy build-ZSBS-Desktop_Qt_5_4_1_MSVC2013_OpenGL_64bit-Release\App\release\App.exe %folder%
copy App\*.vert %folder%
copy App\*.frag %folder%
copy App\kernels.cl %folder%
copy App\montageHeader.cl %folder%

:: icon
copy App\edit.png %folder%

:: clFFT lib
copy App\clFFT.dll %folder%

:: Qt dlls
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Core.dll %folder%
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Gui.dll %folder%
copy C:\Qt\5.4\msvc2013_64_opengl\bin\icu*.dll %folder%
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Widgets.dll %folder%
mkdir %folder%\platforms
copy C:\Qt\5.4\msvc2013_64_opengl\plugins\platforms\qwindows.dll %folder%\platforms


