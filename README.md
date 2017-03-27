# Alenka
A Visualisation System for Biosignals

### Requirments
* git
* Qt 5.4+ libraries
* OpenCL and OpenGL libraries (If you don't have the hardware for this use AMD APP SDK for a CPU implementation of OpenCL.)
* g++ or Visual C++ compiler

On debian-like systems you can use: `sudo apt install git cmake-gui build-essential`

Install Qt via the installer on their website. Select the "Qt 5.7 msvc2015 64/32-bit" package for Windows or "Desktop gcc 64/32-bit" for Linux.

Visual C++ compiler can be acquired by installing Visual C++ Build Tools 2015. Choose "Custom Installation", and uncheck all options but "Windows 8.1 SDK". If you already have Visual Studio, you probably don't need to install this.

### Build instructions
1. Clone this repo
2. Run download-libraries.sh (using git-bash on Windows), and build the subprojects Alenka-File and Alenka-Signal
3. Copy "build.template" file to "build"
4. Modify build file to suit your system (mainly change paths to libraries)
5. Open "Alenka.pro" in Qt Creator (an IDE that was installed alongside Qt), and setup the project (I recommend changing the working directory in Projects -> Run.)
6. Build the project using the hammer button, and run the program via the green play button
