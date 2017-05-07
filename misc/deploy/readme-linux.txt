You need to install the OpenCL driver (libOpenCL.so) and the implementation
appropriate for your system. If you have a dedicated GPU, install the driver
provided by the manufacturer. That should be enough. On Ubuntu the driver is
provided by a package called "ocl-icd-libopencl1".

For Intel integrated GPUs use a package called "beignet".

With Mesa OpenGL driver (the default for most Linux distributions) you must set
"glSharing = 0". (Apparently it doesn't support the cl_khr_gl_sharing extension
due to a bug.)

If any of these doesn't work for you, you can try AMD APP SDK which provides CPU
only implementation of OpenCL. This is also the only way to run this on
VirtualBox. Don't forget to install the VB Guest Additions and set some decent
performance related settings (number of processors, RAM, video memory etc.).

Use "./Alenka" to launch the program.
Use "./Alenka-AMD" to override the default OpenCL driver so that AMD APP SDK
can be found.

Use "./Alenka --help" to get a list of all the available options.

