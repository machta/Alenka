# Alenka
A Visualisation System for Biosignals

### Requirments
* git
* Qt 5.4+
* Boost Program Options
* OpenCL and OpenGL libraries (if you don't have the hardware for this use AMD APP SDK for a CPU implementation of OpenCL)

On debian-like systems you can use: `sudo apt install git cmake-gui libboost-program-options-dev build-essential`

Install Qt via the installer on their website.

### Build instructions
1. Clone this repo
2. Install libraries using download-libraries.sh and build the subprojects
3. Copy build.template to build
4. Modify build to suit your system (mainly change paths to libraries)
5. Open Alenka.pro in Qt Creator and setup the project (I recommend changing "Working directory" in Run and adding "DEFINES+=NDEBUG" to "Additional arguments" in Build -- Build Steps -- qmake for a release build.)
6. Build using the hammer and run with the green play button
