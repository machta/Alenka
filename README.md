# Alenka
A Visualisation System for Biosignals

### Requirments
* git
* cmake 3.1 (for Ubuntu 14 download the latest version)
* C++14 compiler: g++, clang, or Microsoft Visual C++ 2015
* Qt 5.8 (gcc/MSVC and QtCharts module)
* OpenCL 1.2
* OpenGL 2.0
* Matio library

On Debian-like systems you can use: `sudo apt install git cmake-gui build-essential`

Install Qt via the installer on their website. Select the "Qt 5.8 msvc2015 64/32-bit"
package for Windows, or "Desktop gcc 64/32-bit" for Linux. Also select the "QtCharts" module.

MSVC compiler can be acquired by installing Visual C++ Build Tools 2015. Choose "Custom
Installation", and uncheck all options but "Windows 8.1 SDK". If you already have Visual
Studio, you probably don't need to install this.

If your device doesn't support OpenCL (e.g. when running a Linux guest in VirtualBox),
use AMD APP SDK for a CPU implementation of OpenCL.

On Linux install matio package ("libmatio-dev" for Ubuntu). On Windows Matio is downloaded automatically.

