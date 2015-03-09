echo Deploying to %1

del /q %1\*

:: App and source files
copy build-ZSBS-Desktop_Qt_5_4_0_MSVC2013_OpenGL_64bit-Release\App\release\App.exe %1
copy App\shader.vert %1
copy App\shader.frag %1
copy App\kernels.cl %1

:: clFFT lib
copy App\clFFT.dll %1

:: Qt dlls
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Core.dll 			%1
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Gui.dll 			%1
copy C:\Qt\5.4\msvc2013_64_opengl\bin\icu*.dll 				%1
copy C:\Qt\5.4\msvc2013_64_opengl\bin\Qt5Widgets.dll 			%1
::mkdir 									%1\platforms
::copy C:\Qt\5.4\msvc2013_64_opengl\plugins\platforms\qwindows.dll 	%1\platforms

:: Options
echo uncalibrated = false 				>  %1\options.cfg
echo window = hamming 					>> %1\options.cfg
echo dataFileCacheSize = 500000000 # 500 MB		>> %1\options.cfg
echo gpuMemorySize = 100000000 # 100 MB			>> %1\options.cfg
echo blockSize = 32768 # 2^15				>> %1\options.cfg
echo file = C:\Users\Martin\ZSBS\Test\data\gdf\t00.gdf	>> %1\options.cfg
