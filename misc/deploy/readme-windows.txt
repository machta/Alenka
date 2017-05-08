Visual C++ 2015 redistributable is required. You can get it here:
https://www.microsoft.com/en-US/download/details.aspx?id=48145

You need to install a fairly recent driver for your GPU. You can do this via
Windows Update. This works well for the integrated Intel GPU, but for AMD and
Nvidia cards downloading the driver from their website is usually better.

If your device doesn't support the required version of OpenCL, you can try
AMD APP SDK which provides CPU only implementation of OpenCL.

Use "Alenka.exe" to launch the program from command line or double-click it in
the file explorer.

Use "Alenka.exe --help" to get a list of all the available options.

